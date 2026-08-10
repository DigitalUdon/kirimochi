#include "DatabaseManager.h"

#include "PathUtils.h"
#include "TimeUtils.h"
#include "viewmodels/models/MemoBlockListModel.h"

#include <algorithm>
#include <vector>

#include <QDir>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

DatabaseManager::DatabaseManager() = default;

bool DatabaseManager::init(bool is_test) {
    QString connectionName("SQLiteConnection");
    db = QSqlDatabase::addDatabase("QSQLITE", connectionName);

    QString database_path = "";
    if (is_test) {
        database_path = ":memory:";
    } else {
        database_path = PathUtils::databasePath();
    }

    db.setDatabaseName(database_path);
    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    query.exec("PRAGMA journal_mode = WAL;");
    query.exec("PRAGMA foreign_keys = ON;");

    query.exec("CREATE TABLE IF NOT EXISTS memo_project ("
               "    video_id         TEXT PRIMARY KEY,"
               "    video_title      TEXT,"
               "    last_position_ms INTEGER NOT NULL DEFAULT 0,"
               "    status           TEXT    NOT NULL DEFAULT 'wip',"
               "    created_at       INTEGER NOT NULL,"
               "    updated_at       INTEGER NOT NULL"
               ");");

    query.exec(
        "CREATE TABLE IF NOT EXISTS memo ("
        "    id         INTEGER PRIMARY KEY,"
        "    video_id   TEXT    NOT NULL REFERENCES memo_project(video_id) ON DELETE CASCADE,"
        "    start_ms   INTEGER,"
        "    end_ms     INTEGER NOT NULL,"
        "    label      TEXT,"
        "    body       TEXT,"
        "    created_at INTEGER NOT NULL,"
        "    updated_at INTEGER NOT NULL"
        ");");

    query.exec("CREATE TABLE IF NOT EXISTS draft_project ("
               "    draft_id   TEXT PRIMARY KEY,"
               "    title      TEXT    NOT NULL,"
               "    concept    TEXT,"
               "    status     TEXT    NOT NULL DEFAULT 'wip',"
               "    created_at INTEGER NOT NULL,"
               "    updated_at INTEGER NOT NULL"
               ");");

    query.exec(
        "CREATE TABLE draft_item ("
        "    id           INTEGER PRIMARY KEY,"
        "    draft_id     TEXT NOT NULL REFERENCES draft_project(draft_id) ON DELETE CASCADE,"
        "    sort_order   INTEGER NOT NULL,"
        "    kind         TEXT    NOT NULL,"
        "    source_path  TEXT,"
        "    duration_ms  INTEGER,"
        "    start_ms     INTEGER,"
        "    end_ms       INTEGER,"
        "    label        TEXT,"
        "    body         TEXT,"
        "    se_path      TEXT,"
        "    se_offset_ms INTEGER,"
        "    created_at   INTEGER NOT NULL,"
        "    updated_at   INTEGER NOT NULL"
        ");");

    query.exec("CREATE TABLE IF NOT EXISTS asset ("
               "    id          INTEGER PRIMARY KEY,"
               "    name        TEXT    NOT NULL,"
               "    kind        TEXT    NOT NULL,"
               "    file_path   TEXT    NOT NULL,"
               "    duration_ms INTEGER,"
               "    width       INTEGER,"
               "    height      INTEGER,"
               "    provider    TEXT,"
               "    tags        TEXT,"
               "    created_at  INTEGER NOT NULL,"
               "    updated_at  INTEGER NOT NULL"
               ");");

    query.exec("CREATE TABLE IF NOT EXISTS idea ("
               "    id          INTEGER PRIMARY KEY,"
               "    name        TEXT    NOT NULL,"
               "    description TEXT,"
               "    tags        TEXT,"
               "    created_at  INTEGER NOT NULL,"
               "    updated_at  INTEGER NOT NULL"
               ");");

    return true;
}

std::vector<Project> DatabaseManager::getAllProjects(bool only_memo_projects) {
    QSqlQuery query(db);
    if (!query.exec("SELECT video_id, video_title, status, updated_at FROM memo_project")) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int video_id_index = query.record().indexOf("video_id");
    int video_title_index = query.record().indexOf("video_title");
    int status_index = query.record().indexOf("status");
    int updated_at_index = query.record().indexOf("updated_at");

    std::vector<Project> project_list;
    while (query.next()) {
        Project project;

        project.project_id = query.value(video_id_index).toString();
        project.kind = "Memo";
        project.title = query.value(video_title_index).toString();
        project.status = query.value(status_index).toString();
        project.updated_at = query.value(updated_at_index).toLongLong();

        project_list.push_back(project);
    }

    if (!only_memo_projects) {
        QSqlQuery draft_query(db);
        if (!draft_query.exec("SELECT draft_id, title, status, updated_at FROM draft_project")) {
            qWarning() << "SQL error:" << draft_query.lastError().text();
        }

        int draft_id_index = draft_query.record().indexOf("draft_id");
        int title_index = draft_query.record().indexOf("title");
        int draft_status_index = draft_query.record().indexOf("status");
        int draft_updated_at_index = draft_query.record().indexOf("updated_at");

        while (draft_query.next()) {
            Project project;

            project.project_id = draft_query.value(draft_id_index).toString();
            project.kind = "Draft";
            project.title = draft_query.value(title_index).toString();
            project.status = draft_query.value(draft_status_index).toString();
            project.updated_at = draft_query.value(draft_updated_at_index).toLongLong();

            project_list.push_back(project);
        }
    }

    std::sort(project_list.begin(), project_list.end(), std::greater<>());

    return project_list;
}

void DatabaseManager::insertMemoProject(const QString& video_id, const QString& video_title) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO memo_project (video_id, video_title, created_at, updated_at) VALUES "
                  "(?, ?, ?, ?)");

    query.addBindValue(video_id);
    query.addBindValue(video_title);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(TimeUtils::now());

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

MemoProject DatabaseManager::getMemoProject(const QString& video_id) {
    QSqlQuery query(db);
    query.prepare("SELECT video_title, last_position_ms FROM memo_project WHERE video_id = ?");
    query.addBindValue(video_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int video_title_index = query.record().indexOf("video_title");
    int last_position_ms_index = query.record().indexOf("last_position_ms");

    MemoProject memo_project;

    if (query.next()) {
        memo_project.video_id = video_id;
        memo_project.video_title = query.value(video_title_index).toString();
        memo_project.last_position_ms = query.value(last_position_ms_index).toInt();
    }

    return memo_project;
}

void DatabaseManager::updateMemoProjectLastPosition(const QString& video_id, qint32 position) {
    QSqlQuery query(db);
    query.prepare(
        "UPDATE memo_project SET last_position_ms = ?, updated_at = ? WHERE video_id = ?");
    query.addBindValue(position);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(video_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::deleteMemoProject(const QString& video_id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM memo_project WHERE video_id = ?");
    query.addBindValue(video_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::updateMemoProject(const QString& video_id, const QString& title) {
    QSqlQuery query(db);
    query.prepare("UPDATE memo_project SET video_title = ?, updated_at = ? WHERE video_id = ?");

    query.addBindValue(title);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(video_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

qint64 DatabaseManager::insertMemo(const QString& video_id,
                                   qint32 start_ms,
                                   qint32 end_ms,
                                   const QString& label) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO memo (video_id, start_ms, end_ms, label, created_at, updated_at) "
                  "VALUES (?, ?, ?, ?, ?, ?)");

    query.addBindValue(video_id);
    query.addBindValue(start_ms);
    query.addBindValue(end_ms);
    query.addBindValue(label);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(TimeUtils::now());

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    return query.lastInsertId().toLongLong();
}

void DatabaseManager::deleteMemo(qint64 memo_id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM memo WHERE id = ?");
    query.addBindValue(memo_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::updateMemoComment(qint64 memo_id, const QString& body) {
    QSqlQuery query(db);
    query.prepare("UPDATE memo SET body = ?, updated_at = ? WHERE id = ?");
    query.addBindValue(body);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(memo_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::updateMemoStart(qint64 memo_id, qint32 ms) {
    QSqlQuery query(db);
    query.prepare("UPDATE memo SET start_ms = ?, updated_at = ? WHERE id = ?");
    query.addBindValue(ms);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(memo_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::updateMemoEnd(qint64 memo_id, qint32 ms) {
    QSqlQuery query(db);
    query.prepare("UPDATE memo SET end_ms = ?, updated_at = ? WHERE id = ?");
    query.addBindValue(ms);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(memo_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

std::vector<MemoBlock> DatabaseManager::getMemos(const QString& video_id) {
    QSqlQuery query(db);
    query.prepare("SELECT id, start_ms, end_ms, label, body FROM memo WHERE video_id = ?");
    query.addBindValue(video_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int id_index = query.record().indexOf("id");
    int start_ms_index = query.record().indexOf("start_ms");
    int end_ms_index = query.record().indexOf("end_ms");
    int label_index = query.record().indexOf("label");
    int body_index = query.record().indexOf("body");

    std::vector<MemoBlock> memo_block_list;
    while (query.next()) {
        MemoBlock memo;
        memo.memo_id = query.value(id_index).toLongLong();
        memo.start_ms = query.value(start_ms_index).toInt();
        memo.end_ms = query.value(end_ms_index).toInt();
        memo.label = query.value(label_index).toString();
        memo.body = query.value(body_index).toString();

        memo_block_list.push_back(memo);
    }

    std::sort(memo_block_list.begin(), memo_block_list.end());

    return memo_block_list;
}

QString DatabaseManager::getMemoVideoId(qint64 memo_id) {
    QSqlQuery query(db);
    query.prepare("SELECT video_id FROM memo WHERE id = ?");
    query.addBindValue(memo_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int video_id_index = query.record().indexOf("video_id");

    if (query.next()) {
        return query.value(video_id_index).toString();
    }

    return "";
}

void DatabaseManager::insertDraftProject(const QString& draft_id,
                                         const QString& title,
                                         const QString& concept) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO draft_project (draft_id, title, concept, created_at, updated_at) VALUES "
        "(?, ?, ?, ?, ?)");

    query.addBindValue(draft_id);
    query.addBindValue(title);
    query.addBindValue(concept);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(TimeUtils::now());

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

DraftProject DatabaseManager::getDraftProject(const QString& draft_id) {
    QSqlQuery query(db);
    query.prepare("SELECT title, concept FROM draft_project WHERE draft_id = ?");
    query.addBindValue(draft_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int title_index = query.record().indexOf("title");
    int concept_index = query.record().indexOf("concept");

    DraftProject draft_project;

    if (query.next()) {
        draft_project.draft_id = draft_id;
        draft_project.title = query.value(title_index).toString();
        draft_project.concept = query.value(concept_index).toString();
    }

    return draft_project;
}

void DatabaseManager::deleteDraftProject(const QString& draft_id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM draft_project WHERE draft_id = ?");
    query.addBindValue(draft_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::updateDraftProject(const QString& draft_id, const QString& title) {
    QSqlQuery query(db);
    query.prepare("UPDATE draft_project SET title = ?, updated_at = ? WHERE draft_id = ?");

    query.addBindValue(title);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(draft_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

std::vector<QString> DatabaseManager::getAllDraftId() {
    QSqlQuery query(db);
    query.prepare("SELECT draft_id FROM draft_project");

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int draft_id_index = query.record().indexOf("draft_id");

    std::vector<QString> draft_ids;

    while (query.next()) {
        draft_ids.push_back(query.value(draft_id_index).toString());
    }

    return draft_ids;
}

qint64 DatabaseManager::insertDraftItem(const QString& draft_id,
                                        qint32 sort_order,
                                        const QString& kind,
                                        const QString& source_path,
                                        qint32 duration_ms,
                                        qint32 start_ms,
                                        qint32 end_ms,
                                        const QString& label,
                                        const QString& body,
                                        const QString& se_path,
                                        qint32 se_offset_ms) {
    QSqlQuery query(db);
    query.prepare(
        "INSERT INTO draft_item (draft_id, sort_order, kind, source_path, duration_ms, "
        "start_ms, end_ms, label, body, se_path, se_offset_ms, created_at, updated_at) VALUES "
        "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(draft_id);
    query.addBindValue(sort_order);
    query.addBindValue(kind);
    query.addBindValue(source_path);
    query.addBindValue(duration_ms);
    query.addBindValue(start_ms);
    query.addBindValue(end_ms);
    query.addBindValue(label);
    query.addBindValue(body);
    query.addBindValue(se_path);
    query.addBindValue(se_offset_ms);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(TimeUtils::now());

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    return query.lastInsertId().toLongLong();
}

void DatabaseManager::updateDraftItem(qint64 draft_block_id,
                                      const QString& kind,
                                      const QString& source_path,
                                      qint32 duration_ms,
                                      qint32 start_ms,
                                      qint32 end_ms,
                                      const QString& label,
                                      const QString& body,
                                      const QString& se_path,
                                      qint32 se_offset_ms) {
    QSqlQuery query(db);
    query.prepare("UPDATE draft_item SET kind = ?, source_path = ?, duration_ms = ?, "
                  "start_ms = ?, end_ms = ?, label = ?, body = ?, se_path = ?, se_offset_ms = ?, "
                  "updated_at = ? WHERE id = ?");

    query.addBindValue(kind);
    query.addBindValue(source_path);
    query.addBindValue(duration_ms);
    query.addBindValue(start_ms);
    query.addBindValue(end_ms);
    query.addBindValue(label);
    query.addBindValue(body);
    query.addBindValue(se_path);
    query.addBindValue(se_offset_ms);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(draft_block_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

std::vector<DraftBlock> DatabaseManager::getDraftItems(const QString& draft_id) {
    QSqlQuery query(db);
    query.prepare("SELECT id, kind, source_path, duration_ms, start_ms, end_ms, label, body, "
                  "se_path, se_offset_ms FROM draft_item WHERE draft_id = ? ORDER BY sort_order");
    query.addBindValue(draft_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int id_index = query.record().indexOf("id");
    int kind_index = query.record().indexOf("kind");
    int source_path_index = query.record().indexOf("source_path");
    int duration_index = query.record().indexOf("duration_ms");
    int start_ms_index = query.record().indexOf("start_ms");
    int end_ms_index = query.record().indexOf("end_ms");
    int label_index = query.record().indexOf("label");
    int body_index = query.record().indexOf("body");
    int se_path_index = query.record().indexOf("se_path");
    int se_offset_ms_index = query.record().indexOf("se_offset_ms");

    std::vector<DraftBlock> draft_block_list;

    while (query.next()) {
        DraftBlock block;
        block.draft_block_id = query.value(id_index).toLongLong();
        block.kind = query.value(kind_index).toString();
        block.source_path = query.value(source_path_index).toString();
        block.duration = query.value(duration_index).toInt();
        block.start_ms = query.value(start_ms_index).toInt();
        block.end_ms = query.value(end_ms_index).toInt();
        block.label = query.value(label_index).toString();
        block.body = query.value(body_index).toString();
        block.se_path = query.value(se_path_index).toString();
        block.se_offset_ms = query.value(se_offset_ms_index).toInt();

        draft_block_list.push_back(block);
    }

    return draft_block_list;
}

bool DatabaseManager::updateDraftItemSortOrder(std::vector<qint64>& ordered_ids) {
    qDebug() << ordered_ids.size();
    db.transaction();

    QSqlQuery query(db);
    query.prepare("UPDATE draft_item SET sort_order = ? WHERE id = ?");

    for (int i = 0; i < ordered_ids.size(); ++i) {
        query.addBindValue(i);
        query.addBindValue(ordered_ids[i]);
        if (!query.exec()) {
            qWarning() << "SQL error:" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    return db.commit();
}

void DatabaseManager::deleteDraftItem(qint64 draft_block_id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM draft_item WHERE id = ?");
    query.addBindValue(draft_block_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

qint64 DatabaseManager::insertAsset(const QString& name,
                                    const QString& kind,
                                    const QString& file_path,
                                    qint32 duration_ms,
                                    qint32 width,
                                    qint32 height,
                                    const QString& provider,
                                    const QString& tags) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO asset (name, kind, file_path, duration_ms, width, height, "
                  "provider, tags, created_at, updated_at) VALUES "
                  "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(name);
    query.addBindValue(kind);
    query.addBindValue(file_path);
    query.addBindValue(duration_ms);
    query.addBindValue(width);
    query.addBindValue(height);
    query.addBindValue(provider);
    query.addBindValue(tags);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(TimeUtils::now());

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    return query.lastInsertId().toLongLong();
}

std::vector<Asset> DatabaseManager::getAllAssets() {
    QSqlQuery query(db);

    if (!query.exec("SELECT id, name, kind, file_path, duration_ms, width, height, provider, tags, "
                    "updated_at FROM asset")) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int id_index = query.record().indexOf("id");
    int name_index = query.record().indexOf("name");
    int kind_index = query.record().indexOf("kind");
    int file_path_index = query.record().indexOf("file_path");
    int duration_ms_index = query.record().indexOf("duration_ms");
    int width_index = query.record().indexOf("width");
    int height_index = query.record().indexOf("height");
    int provider_index = query.record().indexOf("provider");
    int tags_index = query.record().indexOf("tags");
    int updated_at_index = query.record().indexOf("updated_at");

    std::vector<Asset> asset_list;
    while (query.next()) {
        Asset asset;

        asset.asset_id = query.value(id_index).toLongLong();
        asset.name = query.value(name_index).toString();
        asset.kind = query.value(kind_index).toString();
        asset.file_path = query.value(file_path_index).toString();
        asset.duration_ms = query.value(duration_ms_index).toInt();
        asset.width = query.value(width_index).toInt();
        asset.height = query.value(height_index).toInt();
        asset.provider = query.value(provider_index).toString();
        asset.tags = query.value(tags_index).toString();
        asset.updated_at = query.value(updated_at_index).toLongLong();

        asset_list.push_back(asset);
    }

    std::sort(asset_list.begin(), asset_list.end());

    return asset_list;
}

Asset DatabaseManager::getAsset(qint64 asset_id) {
    QSqlQuery query(db);
    query.prepare("SELECT id, name, kind, file_path, duration_ms, width, height, provider, tags, "
                  "updated_at FROM asset WHERE id = ?");
    query.addBindValue(asset_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int name_index = query.record().indexOf("name");
    int kind_index = query.record().indexOf("kind");
    int file_path_index = query.record().indexOf("file_path");
    int duration_ms_index = query.record().indexOf("duration_ms");
    int width_index = query.record().indexOf("width");
    int height_index = query.record().indexOf("height");
    int provider_index = query.record().indexOf("provider");
    int tags_index = query.record().indexOf("tags");
    int updated_at_index = query.record().indexOf("updated_at");

    Asset asset;

    if (query.next()) {
        asset.asset_id = asset_id;
        asset.name = query.value(name_index).toString();
        asset.kind = query.value(kind_index).toString();
        asset.file_path = query.value(file_path_index).toString();
        asset.duration_ms = query.value(duration_ms_index).toInt();
        asset.width = query.value(width_index).toInt();
        asset.height = query.value(height_index).toInt();
        asset.provider = query.value(provider_index).toString();
        asset.tags = query.value(tags_index).toString();
        asset.updated_at = query.value(updated_at_index).toLongLong();
    }

    return asset;
}

void DatabaseManager::deleteAsset(qint64 asset_id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM asset WHERE id = ?");
    query.addBindValue(asset_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::updateAsset(qint64 asset_id,
                                   const QString& name,
                                   const QString& kind,
                                   const QString& file_path,
                                   qint32 duration_ms,
                                   qint32 width,
                                   qint32 height,
                                   const QString& provider,
                                   const QString& tags) {
    QSqlQuery query(db);
    query.prepare("UPDATE asset SET name = ?, kind = ?, file_path = ?, duration_ms = ?, "
                  "width = ?, height = ?, provider = ?, tags = ?, updated_at = ? "
                  "WHERE id = ?");

    query.addBindValue(name);
    query.addBindValue(kind);
    query.addBindValue(file_path);
    query.addBindValue(duration_ms);
    query.addBindValue(width);
    query.addBindValue(height);
    query.addBindValue(provider);
    query.addBindValue(tags);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(asset_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

qint64
DatabaseManager::insertIdea(const QString& name, const QString& description, const QString& tags) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO idea (name, description, tags, created_at, updated_at) VALUES "
                  "(?, ?, ?, ?, ?)");

    query.addBindValue(name);
    query.addBindValue(description);
    query.addBindValue(tags);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(TimeUtils::now());

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    return query.lastInsertId().toLongLong();
}

std::vector<Idea> DatabaseManager::getAllIdeas() {
    QSqlQuery query(db);
    if (!query.exec("SELECT id, name, description, tags, updated_at FROM idea")) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int id_index = query.record().indexOf("id");
    int name_index = query.record().indexOf("name");
    int description_index = query.record().indexOf("description");
    int tags_index = query.record().indexOf("tags");
    int updated_at_index = query.record().indexOf("updated_at");

    std::vector<Idea> idea_list;
    while (query.next()) {
        Idea idea;

        idea.idea_id = query.value(id_index).toLongLong();
        idea.name = query.value(name_index).toString();
        idea.description = query.value(description_index).toString();
        idea.tags = query.value(tags_index).toString();
        idea.updated_at = query.value(updated_at_index).toLongLong();

        idea_list.push_back(idea);
    }

    std::sort(idea_list.begin(), idea_list.end());

    return idea_list;
}

Idea DatabaseManager::getIdea(qint64 idea_id) {
    QSqlQuery query(db);
    query.prepare("SELECT id, name, description, tags, updated_at FROM idea WHERE id = ?");
    query.addBindValue(idea_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }

    int name_index = query.record().indexOf("name");
    int description_index = query.record().indexOf("description");
    int tags_index = query.record().indexOf("tags");
    int updated_at_index = query.record().indexOf("updated_at");

    Idea idea;

    if (query.next()) {
        idea.idea_id = idea_id;
        idea.name = query.value(name_index).toString();
        idea.description = query.value(description_index).toString();
        idea.tags = query.value(tags_index).toString();
        idea.updated_at = query.value(updated_at_index).toLongLong();
    }

    return idea;
}

void DatabaseManager::deleteIdea(qint64 idea_id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM idea WHERE id = ?");
    query.addBindValue(idea_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}

void DatabaseManager::updateIdea(qint64 idea_id, const QString& name, const QString& description, const QString& tags) {
    QSqlQuery query(db);
    query.prepare("UPDATE idea SET name = ?, description = ?, tags = ?, updated_at = ? "
                  "WHERE id = ?");

    query.addBindValue(name);
    query.addBindValue(description);
    query.addBindValue(tags);
    query.addBindValue(TimeUtils::now());
    query.addBindValue(idea_id);

    if (!query.exec()) {
        qWarning() << "SQL error:" << query.lastError().text();
    }
}
