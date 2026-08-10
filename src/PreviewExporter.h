#pragma once

#include "PathUtils.h"
#include "viewmodels/models/DraftBlockListModel.h"

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QVector>
#include <memory>

namespace PreviewRender {

struct Segment {
    QString filePath;
    bool isImage = false;
    qint64 startMs = 0;
    qint64 endMs = 0;
    qint64 durationMs = 0;
};

struct Se {
    QString filePath;
    qint64 absoluteDelayMs = 0;
};

struct Plan {
    QVector<Segment> segments;
    QVector<Se> ses;
    int outWidth = 1280;
    int outHeight = 720;
    int outFps = 30;
};

inline QString msToSec(qint64 ms) {
    return QString::number(static_cast<double>(ms) / 1000.0, 'f', 3);
}

inline Plan buildPlan(const QVector<DraftBlock>& blocks, const QDir& archiveDir) {
    Plan plan;
    qint64 cumulativeMs = 0;

    for (const DraftBlock& b : blocks) {
        Segment seg;

        if (b.kind == QStringLiteral("memo")) {
            seg.filePath = PathUtils::resolvePathFromVideoId(
                b.source_path, archiveDir, QStringLiteral("video"));
            if (seg.filePath.isEmpty()) {
                qWarning() << "preview: video not found for video_id" << b.source_path
                           << "- block skipped";
                continue;
            }
            seg.startMs = b.start_ms;
            seg.endMs = b.end_ms;
            seg.durationMs = b.end_ms - b.start_ms;
        } else if (b.kind == QStringLiteral("video")) {
            seg.filePath = b.source_path;
            seg.startMs = b.start_ms;
            seg.endMs = b.end_ms;
            seg.durationMs = b.end_ms - b.start_ms;
        } else if (b.kind == QStringLiteral("image")) {
            if (b.duration == 0) {
                qWarning() << "preview: duration is 0 - block skipped";
                continue;
            }
            seg.filePath = b.source_path;
            seg.isImage = true;
            seg.durationMs = b.duration;
        } else {
            continue;
        }

        if (!b.se_path.isEmpty()) {
            plan.ses.append({b.se_path, cumulativeMs + b.se_offset_ms});
        }

        plan.segments.append(seg);
        cumulativeMs += seg.durationMs;
    }
    return plan;
}

inline QStringList buildArgs(const Plan& plan, const QString& outputPath) {
    QStringList args;
    args << QStringLiteral("-y");

    for (const Segment& s : plan.segments) {
        if (s.isImage) {
            args << QStringLiteral("-loop") << QStringLiteral("1") << QStringLiteral("-t")
                 << msToSec(s.durationMs);
        }
        args << QStringLiteral("-i") << s.filePath;
    }
    for (const Se& se : plan.ses) {
        args << QStringLiteral("-i") << se.filePath;
    }

    // --- filter_complex ---
    const int n = static_cast<int>(plan.segments.size());
    const QString videoCommon = QStringLiteral("scale=%1:%2,setsar=1,fps=%3,format=yuv420p")
                                    .arg(plan.outWidth)
                                    .arg(plan.outHeight)
                                    .arg(plan.outFps);
    QString fc;
    QString concatInputs;

    for (int i = 0; i < n; ++i) {
        const Segment& s = plan.segments[i];
        if (s.isImage) {
            fc += QStringLiteral("[%1:v]%2[v%1];").arg(i).arg(videoCommon);
            fc += QStringLiteral("anullsrc=r=48000:cl=stereo,atrim=end=%1[a%2];")
                      .arg(msToSec(s.durationMs))
                      .arg(i);
        } else {
            fc += QStringLiteral("[%1:v]trim=start=%2:end=%3,setpts=PTS-STARTPTS,%4[v%1];")
                      .arg(i)
                      .arg(msToSec(s.startMs))
                      .arg(msToSec(s.endMs))
                      .arg(videoCommon);
            fc += QStringLiteral("[%1:a]atrim=start=%2:end=%3,asetpts=PTS-STARTPTS,"
                                 "aresample=48000,"
                                 "aformat=sample_fmts=fltp:channel_layouts=stereo[a%1];")
                      .arg(i)
                      .arg(msToSec(s.startMs))
                      .arg(msToSec(s.endMs));
        }
        concatInputs += QStringLiteral("[v%1][a%1]").arg(i);
    }

    const bool hasSe = !plan.ses.isEmpty();
    fc += concatInputs + QStringLiteral("concat=n=%1:v=1:a=1[vout]%2")
                             .arg(n)
                             .arg(hasSe ? QStringLiteral("[cat_a]") : QStringLiteral("[aout]"));

    if (hasSe) {
        fc += QStringLiteral(";");
        QString amixInputs = QStringLiteral("[cat_a]");
        for (int i = 0; i < plan.ses.size(); ++i) {
            const int inputIndex = n + i;
            fc += QStringLiteral("[%1:a]adelay=%2|%2[se%3];")
                      .arg(inputIndex)
                      .arg(plan.ses[i].absoluteDelayMs)
                      .arg(i);
            amixInputs += QStringLiteral("[se%1]").arg(i);
        }
        fc += amixInputs +
              QStringLiteral("amix=inputs=%1:duration=first[aout]").arg(plan.ses.size() + 1);
    }

    args << QStringLiteral("-filter_complex") << fc << QStringLiteral("-map")
         << QStringLiteral("[vout]") << QStringLiteral("-map") << QStringLiteral("[aout]")
         << QStringLiteral("-c:v") << QStringLiteral("libx264") << QStringLiteral("-preset")
         << QStringLiteral("ultrafast") << QStringLiteral("-c:a") << QStringLiteral("aac")
         << outputPath;
    return args;
}

} // namespace PreviewRender

class PreviewExporter : public QObject {
    Q_OBJECT
  public:
    explicit PreviewExporter(QObject* parent = nullptr) : QObject(parent) {
    }

    bool isRunning() const {
        return m_process && m_process->state() != QProcess::NotRunning;
    }

    void exportPreview(const PreviewRender::Plan& plan) {
        if (plan.segments.isEmpty()) {
            qDebug() << "preview: no renderable blocks, nothing to do";
            return;
        }
        if (isRunning()) {
            qDebug() << "preview: export already running, request ignored";
            return;
        }

        if (!m_tempDir) {
            m_tempDir = std::make_unique<QTemporaryDir>();
            if (!m_tempDir->isValid()) {
                qDebug() << "preview: failed to create temp dir";
                emit exportFailed();
                return;
            }
        }
        const QString outputPath =
            m_tempDir->filePath(QStringLiteral("preview_%1.mp4").arg(++m_exportCount));

        const QStringList args = PreviewRender::buildArgs(plan, outputPath);
        qDebug() << "preview: ffmpeg args:" << args;

        m_process = std::make_unique<QProcess>();
        connect(m_process.get(),
                &QProcess::finished,
                this,
                [this, outputPath](int exitCode, QProcess::ExitStatus status) {
                    if (status == QProcess::NormalExit && exitCode == 0) {
                        emit exportFinished(QUrl::fromLocalFile(outputPath).toString());
                    } else {
                        qDebug() << "preview: ffmpeg failed:" << m_process->readAllStandardError();
                        emit exportFailed();
                    }
                });
        m_process->start(QStringLiteral("ffmpeg"), args);
    }

  signals:
    void exportFinished(const QString& outputPath);
    void exportFailed();

  private:
    std::unique_ptr<QTemporaryDir> m_tempDir;
    std::unique_ptr<QProcess> m_process;
    int m_exportCount = 0;
};
