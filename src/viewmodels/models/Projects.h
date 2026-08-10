#pragma once

#include <QString>

struct MemoProject {
    QString video_id = "";
    QString video_title = "";
    qint32 last_position_ms = 0;
};

struct DraftProject {
    QString draft_id = "";
    QString title = "";
    QString concept = "";
};
