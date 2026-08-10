#pragma once

#include "PathUtils.h"
#include "SrtUtils.h"
#include "viewmodels/models/DraftBlockListModel.h"

#include <algorithm>
#include <cmath>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QProcess>
#include <QString>
#include <QTextStream>
#include <QUrl>
#include <QVector>

namespace DraftExport {

constexpr int kFps = 30;
constexpr int kWidth = 1280;
constexpr int kHeight = 720;

struct Clip {
    QString filePath;
    bool isImage = false;
    qint64 timelineStartMs = 0;
    qint64 inMs = 0;
    qint64 outMs = 0;
    qint64 durationMs = 0;
};

struct SeClip {
    QString filePath;
    qint64 timelineStartMs = 0;
    qint64 durationMs = 0;
};

struct SubtitleCue {
    qint64 timelineStartMs = 0;
    qint64 timelineEndMs = 0;
    QString text;
};

struct Timeline {
    QVector<Clip> clips;
    QVector<SeClip> seClips;
    QVector<SubtitleCue> subtitles;
};

inline qint64 probeDurationMs(const QString& filePath) {
    QProcess process;
    process.start("ffprobe",
                   {"-v",
                    "error",
                    "-show_entries",
                    "format=duration",
                    "-of",
                    "default=noprint_wrappers=1:nokey=1",
                    filePath});
    process.waitForFinished();

    const QString output = process.readAllStandardOutput().trimmed();
    bool ok = false;
    const double seconds = output.toDouble(&ok);
    if (!ok) {
        qWarning() << "export: failed to probe duration for" << filePath;
        return 0;
    }
    return static_cast<qint64>(seconds * 1000);
}

inline void appendSeIfPresent(Timeline& timeline,
                               const DraftBlock& b,
                               qint64 blockTimelineStartMs) {
    if (b.se_path.isEmpty()) {
        return;
    }
    const qint64 durationMs = probeDurationMs(b.se_path);
    timeline.seClips.append({b.se_path, blockTimelineStartMs + b.se_offset_ms, durationMs});
}

inline void appendSubtitlesForMemoBlock(Timeline& timeline,
                                         const DraftBlock& b,
                                         qint64 blockTimelineStartMs,
                                         const QDir& archiveDir) {
    const QString srtPath =
        PathUtils::resolvePathFromVideoId(b.source_path, archiveDir, QStringLiteral("subtitle"));
    const auto cues = SrtUtils::parseSrt(srtPath);

    for (const auto& cue : cues) {
        if (cue.start_ms >= b.start_ms && cue.end_ms <= b.end_ms) {
            SubtitleCue sub;
            sub.timelineStartMs = blockTimelineStartMs + (cue.start_ms - b.start_ms);
            sub.timelineEndMs = blockTimelineStartMs + (cue.end_ms - b.start_ms);
            sub.text = cue.text;
            timeline.subtitles.append(sub);
        }
    }
}

inline Timeline buildTimeline(const QVector<DraftBlock>& blocks, const QDir& archiveDir) {
    Timeline timeline;
    qint64 cumulativeMs = 0;

    for (const DraftBlock& b : blocks) {
        if (b.kind == QStringLiteral("memo")) {
            const QString filePath = PathUtils::resolvePathFromVideoId(
                b.source_path, archiveDir, QStringLiteral("video"));

            Clip clip;
            clip.filePath = filePath;
            clip.timelineStartMs = cumulativeMs;
            clip.inMs = b.start_ms;
            clip.outMs = b.end_ms;
            clip.durationMs = b.end_ms - b.start_ms;
            timeline.clips.append(clip);

            appendSubtitlesForMemoBlock(timeline, b, cumulativeMs, archiveDir);
            appendSeIfPresent(timeline, b, cumulativeMs);

            cumulativeMs += clip.durationMs;
        } else if (b.kind == QStringLiteral("video")) {
            Clip clip;
            clip.filePath = b.source_path;
            clip.timelineStartMs = cumulativeMs;
            clip.inMs = b.start_ms;
            clip.outMs = b.end_ms;
            clip.durationMs = b.end_ms - b.start_ms;
            timeline.clips.append(clip);

            appendSeIfPresent(timeline, b, cumulativeMs);

            cumulativeMs += clip.durationMs;
        } else if (b.kind == QStringLiteral("image")) {
            if (b.duration == 0) {
                qWarning() << "export: image duration is 0 - block skipped";
                continue;
            }

            Clip clip;
            clip.filePath = b.source_path;
            clip.isImage = true;
            clip.timelineStartMs = cumulativeMs;
            clip.inMs = 0;
            clip.outMs = b.duration;
            clip.durationMs = b.duration;
            timeline.clips.append(clip);

            appendSeIfPresent(timeline, b, cumulativeMs);

            cumulativeMs += clip.durationMs;
        } else {
            continue;
        }
    }

    return timeline;
}

inline qint64 msToFrame(qint64 ms) {
    return static_cast<qint64>(std::llround(static_cast<double>(ms) * kFps / 1000.0));
}

inline QString xmlEscape(const QString& text) {
    QString escaped = text;
    escaped.replace('&', QStringLiteral("&amp;"));
    escaped.replace('<', QStringLiteral("&lt;"));
    escaped.replace('>', QStringLiteral("&gt;"));
    return escaped;
}

inline QString pathToFileUrl(const QString& path) {
    return QUrl::fromLocalFile(path).toString();
}

inline QString buildXml(const Timeline& timeline, const QString& sequenceName) {
    QString xml;
    QTextStream out(&xml);

    qint64 sequenceDurationMs = 0;
    for (const auto& c : timeline.clips) {
        sequenceDurationMs = std::max(sequenceDurationMs, c.timelineStartMs + c.durationMs);
    }
    for (const auto& s : timeline.seClips) {
        sequenceDurationMs = std::max(sequenceDurationMs, s.timelineStartMs + s.durationMs);
    }

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<!DOCTYPE xmeml>\n";
    out << "<xmeml version=\"5\">\n";
    out << "  <sequence>\n";
    out << "    <name>" << xmlEscape(sequenceName) << "</name>\n";
    out << "    <duration>" << msToFrame(sequenceDurationMs) << "</duration>\n";
    out << "    <rate>\n";
    out << "      <timebase>" << kFps << "</timebase>\n";
    out << "      <ntsc>FALSE</ntsc>\n";
    out << "    </rate>\n";
    out << "    <media>\n";
    out << "      <video>\n";
    out << "        <format>\n";
    out << "          <samplecharacteristics>\n";
    out << "            <width>" << kWidth << "</width>\n";
    out << "            <height>" << kHeight << "</height>\n";
    out << "          </samplecharacteristics>\n";
    out << "        </format>\n";
    out << "        <track>\n";

    QMap<QString, QString> fileIds;
    int fileCounter = 0;
    int clipCounter = 0;

    for (const auto& c : timeline.clips) {
        ++clipCounter;
        const QString clipId = QStringLiteral("clipitem-%1").arg(clipCounter);

        QString fileId;
        bool isNewFile = false;
        auto it = fileIds.find(c.filePath);
        if (it != fileIds.end()) {
            fileId = it.value();
        } else {
            ++fileCounter;
            fileId = QStringLiteral("file-%1").arg(fileCounter);
            fileIds.insert(c.filePath, fileId);
            isNewFile = true;
        }

        const QFileInfo fi(c.filePath);

        out << "          <clipitem id=\"" << clipId << "\">\n";
        out << "            <name>" << xmlEscape(fi.fileName()) << "</name>\n";
        out << "            <duration>" << msToFrame(c.durationMs) << "</duration>\n";
        out << "            <rate>\n";
        out << "              <timebase>" << kFps << "</timebase>\n";
        out << "              <ntsc>FALSE</ntsc>\n";
        out << "            </rate>\n";
        out << "            <start>" << msToFrame(c.timelineStartMs) << "</start>\n";
        out << "            <end>" << msToFrame(c.timelineStartMs + c.durationMs) << "</end>\n";
        out << "            <in>" << msToFrame(c.inMs) << "</in>\n";
        out << "            <out>" << msToFrame(c.outMs) << "</out>\n";

        if (isNewFile) {
            out << "            <file id=\"" << fileId << "\">\n";
            out << "              <name>" << xmlEscape(fi.fileName()) << "</name>\n";
            out << "              <pathurl>" << pathToFileUrl(c.filePath) << "</pathurl>\n";
            out << "              <rate>\n";
            out << "                <timebase>" << kFps << "</timebase>\n";
            out << "                <ntsc>FALSE</ntsc>\n";
            out << "              </rate>\n";
            if (c.isImage) {
                out << "              <duration>" << msToFrame(c.durationMs) << "</duration>\n";
            }
            out << "              <media>\n";
            out << "                <video>\n";
            out << "                  <samplecharacteristics>\n";
            out << "                    <width>" << kWidth << "</width>\n";
            out << "                    <height>" << kHeight << "</height>\n";
            out << "                  </samplecharacteristics>\n";
            out << "                </video>\n";
            if (!c.isImage) {
                out << "                <audio/>\n";
            }
            out << "              </media>\n";
            out << "            </file>\n";
        } else {
            out << "            <file id=\"" << fileId << "\"/>\n";
        }

        out << "          </clipitem>\n";
    }

    out << "        </track>\n";
    out << "      </video>\n";

    if (!timeline.seClips.isEmpty()) {
        out << "      <audio>\n";
        out << "        <track>\n";

        int seCounter = 0;
        for (const auto& s : timeline.seClips) {
            ++seCounter;
            const QString clipId = QStringLiteral("seclipitem-%1").arg(seCounter);
            const QString fileId = QStringLiteral("sefile-%1").arg(seCounter);
            const QFileInfo fi(s.filePath);

            out << "          <clipitem id=\"" << clipId << "\">\n";
            out << "            <name>" << xmlEscape(fi.fileName()) << "</name>\n";
            out << "            <duration>" << msToFrame(s.durationMs) << "</duration>\n";
            out << "            <rate>\n";
            out << "              <timebase>" << kFps << "</timebase>\n";
            out << "              <ntsc>FALSE</ntsc>\n";
            out << "            </rate>\n";
            out << "            <start>" << msToFrame(s.timelineStartMs) << "</start>\n";
            out << "            <end>" << msToFrame(s.timelineStartMs + s.durationMs) << "</end>\n";
            out << "            <in>0</in>\n";
            out << "            <out>" << msToFrame(s.durationMs) << "</out>\n";
            out << "            <file id=\"" << fileId << "\">\n";
            out << "              <name>" << xmlEscape(fi.fileName()) << "</name>\n";
            out << "              <pathurl>" << pathToFileUrl(s.filePath) << "</pathurl>\n";
            out << "              <rate>\n";
            out << "                <timebase>" << kFps << "</timebase>\n";
            out << "                <ntsc>FALSE</ntsc>\n";
            out << "              </rate>\n";
            out << "              <media>\n";
            out << "                <audio/>\n";
            out << "              </media>\n";
            out << "            </file>\n";
            out << "          </clipitem>\n";
        }

        out << "        </track>\n";
        out << "      </audio>\n";
    }

    out << "    </media>\n";
    out << "  </sequence>\n";
    out << "</xmeml>\n";

    return xml;
}

inline QString msToSrtTimecode(qint64 ms) {
    const qint64 hours = ms / 3600000;
    ms %= 3600000;
    const qint64 minutes = ms / 60000;
    ms %= 60000;
    const qint64 seconds = ms / 1000;
    const qint64 millis = ms % 1000;

    return QStringLiteral("%1:%2:%3,%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(millis, 3, 10, QChar('0'));
}

inline QString buildSrt(const Timeline& timeline) {
    QString srt;
    QTextStream out(&srt);

    int index = 0;
    for (const auto& cue : timeline.subtitles) {
        ++index;
        out << index << "\n";
        out << msToSrtTimecode(cue.timelineStartMs) << " --> " << msToSrtTimecode(cue.timelineEndMs)
            << "\n";
        out << cue.text << "\n\n";
    }

    return srt;
}

inline bool writeTextFile(const QString& path, const QString& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "export: failed to open file for write:" << path;
        return false;
    }
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << content;
    file.close();
    return true;
}

} // namespace DraftExport
