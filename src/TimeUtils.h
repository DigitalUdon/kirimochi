#pragma once

#include <QDateTime>
#include <QTimeZone>

namespace TimeUtils {
inline QString toJstString(qint64 secs) {
    return QDateTime::fromSecsSinceEpoch(secs, QTimeZone("Asia/Tokyo"))
        .toString("yyyy-MM-dd HH:mm:ss");
}

inline qint64 now() {
    return QDateTime::currentDateTime().toSecsSinceEpoch();
}
} // namespace TimeUtils
