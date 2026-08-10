#pragma once

#include <map>

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QString>

namespace PathUtils {
inline QDir appLocalDataDir() {
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    dir.mkpath(".");
    return dir;
}

inline QString databasePath() {
    return appLocalDataDir().filePath("kirimochi_data.db");
}

/**
 * @param target = "title" | "thumbnail" | "chat" | "video" | "subtitle"
 */
inline QString
resolvePathFromVideoId(const QString& video_id, const QDir& archive_dir, const QString& target) {
    if (video_id.isEmpty()) {
        return "";
    }

    static const std::map<QString, QString> filenames = {{"title", "title.txt"},
                                                         {"thumbnail", "thumbnail.jpg"},
                                                         {"chat", "chat_volume_data.html"},
                                                         {"video", "video."},
                                                         {"subtitle", "transcription.srt"}};
    static const QStringList video_exts = {"mp4", "mkv", "webm"};

    QStringList dirs = archive_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const auto& dir : std::as_const(dirs)) {
        if (!dir.contains(video_id)) {
            continue;
        }

        if (target == "video") {
            QString base = archive_dir.filePath(dir + "/" + filenames.at(target));
            for (const auto& ext : video_exts) {
                QString local_path = base + ext;
                if (QFileInfo::exists(local_path)) {
                    return local_path;
                }
            }
        } else {
            QString local_path = archive_dir.filePath(dir + "/" + filenames.at(target));
            if (QFileInfo::exists(local_path)) {
                return local_path;
            }
        }
        break;
    }
    return "";
}
} // namespace PathUtils
