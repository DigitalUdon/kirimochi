#pragma once

#include "viewmodels/models/AssetListModel.h"
#include "viewmodels/models/DraftBlockListModel.h"
#include "viewmodels/models/IdeaListModel.h"
#include "viewmodels/models/MemoBlockListModel.h"
#include "viewmodels/models/ProjectListModel.h"
#include "viewmodels/models/Projects.h"

#include <QString>
#include <QtSql/QSqlDatabase>

class DatabaseManager {
  public:
    DatabaseManager();
    bool init(bool is_test = false);

    std::vector<Project> getAllProjects(bool only_memo_projects = false);

    void insertMemoProject(const QString& video_id, const QString& video_title);
    MemoProject getMemoProject(const QString& video_id);
    void updateMemoProjectLastPosition(const QString& video_id, qint32 position);
    void deleteMemoProject(const QString& video_id);
    void updateMemoProject(const QString& video_id, const QString& title);

    qint64
    insertMemo(const QString& video_id, qint32 start_ms, qint32 end_ms, const QString& label);
    void deleteMemo(qint64 memo_id);
    void updateMemoComment(qint64 memo_id, const QString& body);
    void updateMemoStart(qint64 memo_id, qint32 ms);
    void updateMemoEnd(qint64 memo_id, qint32 ms);
    std::vector<MemoBlock> getMemos(const QString& video_id);
    QString getMemoVideoId(qint64 memo_id);

    void insertDraftProject(const QString& draft_id, const QString& title, const QString& concept);
    DraftProject getDraftProject(const QString& draft_id);
    void deleteDraftProject(const QString& draft_id);
    void updateDraftProject(const QString& draft_id, const QString& title);

    qint64 insertDraftItem(const QString& draft_id,
                           qint32 sort_order,
                           const QString& kind,
                           const QString& source_path,
                           qint32 duration_ms,
                           qint32 start_ms,
                           qint32 end_ms,
                           const QString& label,
                           const QString& body,
                           const QString& se_path,
                           qint32 se_offset_ms);
    void updateDraftItem(qint64 draft_block_id,
                         const QString& kind,
                         const QString& source_path,
                         qint32 duration_ms,
                         qint32 start_ms,
                         qint32 end_ms,
                         const QString& label,
                         const QString& body,
                         const QString& se_path,
                         qint32 se_offset_ms);
    std::vector<DraftBlock> getDraftItems(const QString& draft_id);
    std::vector<QString> getAllDraftId();
    bool updateDraftItemSortOrder(std::vector<qint64>& ordered_ids);
    void deleteDraftItem(qint64 draft_block_id);

    qint64 insertAsset(const QString& name,
                       const QString& kind,
                       const QString& file_path,
                       qint32 duration_ms = 0,
                       qint32 width = 0,
                       qint32 height = 0,
                       const QString& provider = QString(),
                       const QString& tags = QString());
    std::vector<Asset> getAllAssets();
    Asset getAsset(qint64 asset_id);
    void deleteAsset(qint64 asset_id);
    void updateAsset(qint64 asset_id,
                    const QString& name,
                    const QString& kind,
                    const QString& file_path,
                    qint32 duration_ms,
                    qint32 width,
                    qint32 height,
                    const QString& provider,
                    const QString& tags);

    qint64 insertIdea(const QString& name,
                      const QString& description = QString(),
                      const QString& tags = QString());
    std::vector<Idea> getAllIdeas();
    Idea getIdea(qint64 idea_id);
    void deleteIdea(qint64 idea_id);
    void updateIdea(qint64 idea_id, const QString& name, const QString& description, const QString& tags);

  private:
    QSqlDatabase db;
};
