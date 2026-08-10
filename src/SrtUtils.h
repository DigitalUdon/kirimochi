#pragma once

#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <vector>

struct SrtCue {
    qint32 index;
    qint32 start_ms;
    qint32 end_ms;
    QString text;
};

namespace SrtUtils {
inline qint32 parseTimecode(const QString& timecode) {
    const QStringList hms = timecode.split(':');
    if (hms.size() != 3) {
        return -1;
    }

    QString sec_part = hms[2];
    sec_part.replace('.', ',');
    const QStringList sec_ms = sec_part.split(',');
    if (sec_ms.size() != 2) {
        return -1;
    }

    bool ok_h = false;
    bool ok_m = false;
    bool ok_s = false;
    bool ok_ms = false;
    const qint32 hours = hms[0].toInt(&ok_h);
    const qint32 minutes = hms[1].toInt(&ok_m);
    const qint32 seconds = sec_ms[0].toInt(&ok_s);
    const qint32 millis = sec_ms[1].toInt(&ok_ms);

    if (!ok_h || !ok_m || !ok_s || !ok_ms) {
        return -1;
    }

    return ((hours * 60 + minutes) * 60 + seconds) * 1000 + millis;
}

inline std::vector<SrtCue> parseSrt(const QString& file_path) {
    std::vector<SrtCue> cues;

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return cues;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    content.remove(QChar(0xFEFF)); // BOM
    content.replace("\r\n", "\n");
    content.replace('\r', '\n');
    const QStringList lines = content.split('\n');

    int i = 0;
    const int line_count = lines.size();

    while (i < line_count) {
        while (i < line_count && lines[i].trimmed().isEmpty()) {
            ++i;
        }
        if (i >= line_count) {
            break;
        }

        qint32 index = static_cast<qint32>(cues.size()) + 1;
        if (!lines[i].contains("-->")) {
            bool ok = false;
            const qint32 parsed = lines[i].trimmed().toInt(&ok);
            if (ok) {
                index = parsed;
            }
            ++i;
        }

        if (i >= line_count || !lines[i].contains("-->")) {
            while (i < line_count && !lines[i].trimmed().isEmpty()) {
                ++i;
            }
            continue;
        }

        const QStringList parts = lines[i].split("-->");
        const qint32 start_ms = parseTimecode(parts.value(0).trimmed());
        const qint32 end_ms = parseTimecode(parts.value(1).trimmed().section(' ', 0, 0));
        ++i;

        QStringList text_lines;
        while (i < line_count && !lines[i].trimmed().isEmpty()) {
            text_lines << lines[i];
            ++i;
        }

        if (start_ms < 0 || end_ms < 0) {
            continue;
        }

        cues.push_back({index, start_ms, end_ms, text_lines.join('\n')});
    }

    return cues;
}

} // namespace SrtUtils
