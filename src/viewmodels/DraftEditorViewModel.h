#pragma once

#include "DatabaseManager.h"
#include "DraftExporter.h"
#include "PathUtils.h"
#include "PreviewExporter.h"
#include "SettingModel.h"
#include "SrtUtils.h"
#include "viewmodels/models/DraftBlockListModel.h"
#include "viewmodels/models/ProjectListModel.h"
#include "viewmodels/models/Projects.h"
#include "viewmodels/models/SrtCueListModel.h"

#include <algorithm>
#include <vector>

#include <QDebug>
#include <QDir>
#include <QObject>
#include <QProcess>
#include <QRandomGenerator>
#include <QtQmlIntegration/qqmlintegration.h>

class DraftEditorViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created only from C++")
    Q_PROPERTY(DraftBlockListModel* draft_block_list MEMBER draft_block_list CONSTANT)
    Q_PROPERTY(ProjectListModel* project_list MEMBER project_list CONSTANT)
    Q_PROPERTY(MemoBlockListModel* memo_block_list MEMBER memo_block_list CONSTANT)
    Q_PROPERTY(SrtCueListModel* srt_cue_list MEMBER srt_cue_list CONSTANT)
    Q_PROPERTY(qint64 pendingSeTargetId READ pendingSeTargetId NOTIFY pendingSeTargetIdChanged)
  public:
    explicit DraftEditorViewModel(DatabaseManager& db,
                                  const SettingModel& setting,
                                  const QString& title,
                                  const QString& concept,
                                  const QString& draft_id,
                                  QObject* parent = nullptr)
        : QObject(parent) {
        m_db = &db;
        m_setting = &setting;

        m_title = title;
        m_concept = concept;

        draft_block_list = new DraftBlockListModel(this);

        auto draft_project = m_db->getDraftProject(draft_id);
        if (draft_project.draft_id.isEmpty()) {
            qDebug() << "Create new draft project.";
            m_draft_id = generateDraftId();
            m_db->insertDraftProject(m_draft_id, m_title, m_concept);
        } else {
            qDebug() << "Open exiting project";
            m_draft_id = draft_id;
            draft_block_list->setDraftBlocks(m_db->getDraftItems(m_draft_id));
        }

        project_list = new ProjectListModel(this);
        auto projects = m_db->getAllProjects(true);
        for (auto& project : projects) {
            project.thumbnail_path = PathUtils::resolvePathFromVideoId(
                project.project_id, m_setting->archiveDir(), "thumbnail");
        }
        project_list->setProjects(projects);

        connect(&m_exporter,
                &PreviewExporter::exportFinished,
                this,
                &DraftEditorViewModel::previewReady);
        
        srt_cue_list = new SrtCueListModel(this);
    }

    Q_INVOKABLE void addProject(const QString& project_id) {
        m_selected_projects.push_back(MemoProject{project_id, QString(), 0});

        memo_block_list = new MemoBlockListModel(this);
        memo_block_list->setMemoBlocks(m_db->getMemos(project_id));
    }

    Q_INVOKABLE void addMemo(
        qint64 memo_id, qint32 start_ms, qint32 end_ms, const QString& label, const QString& body) {
        const qint32 sort_order = draft_block_list->rowCount(QModelIndex());

        auto video_id = m_db->getMemoVideoId(memo_id);
        const qint64 draft_block_id = m_db->insertDraftItem(m_draft_id,
                                                            sort_order,
                                                            "memo",
                                                            video_id,
                                                            0,
                                                            start_ms,
                                                            end_ms,
                                                            label,
                                                            body,
                                                            QString(),
                                                            0);

        draft_block_list->add(
            draft_block_id, "memo", QString(), 0, start_ms, end_ms, label, body, QString(), 0);
    }

    Q_INVOKABLE void addVideo(qint64 asset_id) {
        const qint32 sort_order = draft_block_list->rowCount(QModelIndex());

        auto video = m_db->getAsset(asset_id);
        const qint64 draft_block_id = m_db->insertDraftItem(m_draft_id,
                                                            sort_order,
                                                            "video",
                                                            video.file_path,
                                                            video.duration_ms,
                                                            0,
                                                            video.duration_ms,
                                                            QString(),
                                                            QString(),
                                                            QString(),
                                                            0);

        draft_block_list->add(draft_block_id,
                              "video",
                              video.file_path,
                              video.duration_ms,
                              0,
                              video.duration_ms,
                              QString(),
                              QString(),
                              QString(),
                              0);
    }

    Q_INVOKABLE void addImage(qint64 asset_id) {
        const qint32 sort_order = draft_block_list->rowCount(QModelIndex());

        auto image = m_db->getAsset(asset_id);
        const qint64 draft_block_id = m_db->insertDraftItem(m_draft_id,
                                                            sort_order,
                                                            "image",
                                                            image.file_path,
                                                            1000,
                                                            0,
                                                            0,
                                                            QString(),
                                                            QString(),
                                                            QString(),
                                                            0);

        draft_block_list->add(
            draft_block_id, "image", image.file_path, 0, 0, 0, QString(), QString(), QString(), 0);
    }

    Q_INVOKABLE void addIdea(qint64 idea_id) {
        const qint32 sort_order = draft_block_list->rowCount(QModelIndex());

        auto idea = m_db->getIdea(idea_id);
        const qint64 draft_block_id = m_db->insertDraftItem(m_draft_id,
                                                            sort_order,
                                                            "idea",
                                                            QString(),
                                                            0,
                                                            0,
                                                            0,
                                                            QString(),
                                                            idea.description,
                                                            QString(),
                                                            0);

        draft_block_list->add(
            draft_block_id, "idea", QString(), 0, 0, 0, QString(), idea.description, QString(), 0);
    }

    Q_INVOKABLE void deleteDraftBlock(qint64 draft_block_id) {
        draft_block_list->remove(draft_block_id);
        m_db->deleteDraftItem(draft_block_id);
    }

    Q_INVOKABLE void duplicateDraftBlock(qint64 draft_block_id) {
        DraftBlock block = draft_block_list->blockAt(draft_block_id);

        const qint32 sort_order = draft_block_list->rowCount(QModelIndex());
        const qint64 new_draft_block_id = m_db->insertDraftItem(m_draft_id,
                                                                sort_order,
                                                                block.kind,
                                                                block.source_path,
                                                                block.duration,
                                                                block.start_ms,
                                                                block.end_ms,
                                                                block.label,
                                                                block.body,
                                                                block.se_path,
                                                                block.se_offset_ms);

        block.draft_block_id = new_draft_block_id;
        draft_block_list->insertAfter(draft_block_id, block);

        commit_draft_block_order();
    }

    Q_INVOKABLE void move(qint32 from, qint32 to) {
        draft_block_list->move(from, to);
    }

    Q_INVOKABLE bool commit_draft_block_order() {
        auto ordered_ids = draft_block_list->getOrderedIds();
        return m_db->updateDraftItemSortOrder(ordered_ids);
    }

    [[nodiscard]] qint64 pendingSeTargetId() const {
        return m_pending_se_target_id;
    }

    Q_INVOKABLE void setPendingSeTargetId(qint64 target_id) {
        m_pending_se_target_id = target_id;
        emit pendingSeTargetIdChanged();
    }

    Q_INVOKABLE void attachSeToBlock(qint64 asset_id) {
        DraftBlock block = draft_block_list->blockAt(m_pending_se_target_id);
        auto se = m_db->getAsset(asset_id);
        block.se_path = se.file_path;
        block.se_offset_ms = 0; // TODO: hardcode

        m_db->updateDraftItem(block.draft_block_id,
                              block.kind,
                              block.source_path,
                              block.duration,
                              block.start_ms,
                              block.end_ms,
                              block.label,
                              block.body,
                              block.se_path,
                              block.se_offset_ms);

        draft_block_list->attachSe(m_pending_se_target_id, se.file_path);

        m_pending_se_target_id = -1;
        emit pendingSeTargetIdChanged();
    }

    Q_INVOKABLE void resetSeFromBlock(qint64 target_id) {
        DraftBlock block = draft_block_list->blockAt(target_id);
        block.se_path = "";
        block.se_offset_ms = 0; // TODO: hardcode

        m_db->updateDraftItem(block.draft_block_id,
                              block.kind,
                              block.source_path,
                              block.duration,
                              block.start_ms,
                              block.end_ms,
                              block.label,
                              block.body,
                              block.se_path,
                              block.se_offset_ms);

        draft_block_list->removeSe(target_id);
    }

    Q_INVOKABLE void updateBlockDuration(qint64 draft_block_id, qint32 duration_ms) {
        DraftBlock block = draft_block_list->blockAt(draft_block_id);
        block.duration = duration_ms;

        m_db->updateDraftItem(block.draft_block_id,
                              block.kind,
                              block.source_path,
                              block.duration,
                              block.start_ms,
                              block.end_ms,
                              block.label,
                              block.body,
                              block.se_path,
                              block.se_offset_ms);

        draft_block_list->updateDuration(draft_block_id, duration_ms);
    }

    Q_INVOKABLE void updateBlockRange(qint64 draft_block_id, qint32 start_ms, qint32 end_ms) {
        DraftBlock block = draft_block_list->blockAt(draft_block_id);
        block.start_ms = start_ms;
        block.end_ms = end_ms;

        m_db->updateDraftItem(block.draft_block_id,
                              block.kind,
                              block.source_path,
                              block.duration,
                              block.start_ms,
                              block.end_ms,
                              block.label,
                              block.body,
                              block.se_path,
                              block.se_offset_ms);

        draft_block_list->updateRange(draft_block_id, start_ms, end_ms);
    }

    Q_INVOKABLE void openRangeDialog(qint64 draft_block_id) {
        const DraftBlock block = draft_block_list->blockAt(draft_block_id);

        if (block.source_path != m_srt_video_id) {
            const QString srt_path = PathUtils::resolvePathFromVideoId(
                block.source_path, m_setting->archiveDir(), "subtitle");
            m_srt_cues = SrtUtils::parseSrt(srt_path);
            m_srt_video_id = block.source_path;
        }

        const qint32 range_start = std::max(0, block.start_ms - 180000);
        const qint32 range_end = block.end_ms + 180000;

        std::vector<SrtCue> in_range;
        for (const auto& cue : m_srt_cues) {
            if (cue.end_ms >= range_start && cue.start_ms <= range_end) {
                in_range.push_back(cue);
            }
        }

        srt_cue_list->setCues(in_range);
    }

    Q_INVOKABLE void confirmRange(qint64 draft_block_id) {
        const auto range = srt_cue_list->getSelectedRange();

        if (range.valid) {
            const qint32 start_ms = std::max(0, range.start_ms - 1000);
            const qint32 end_ms = range.end_ms + 1000;
            updateBlockRange(draft_block_id, start_ms, end_ms);
        }

        srt_cue_list->reset();
    }

    Q_INVOKABLE void requestPreview() {
        const auto plan =
            PreviewRender::buildPlan(draft_block_list->blocks(), m_setting->archiveDir());
        m_exporter.exportPreview(plan);
    }

    Q_INVOKABLE QString mediaDurationAsString(const QUrl& path) {
        auto duration_ms = getVideoDurationMs(path.toLocalFile());

        return QTime(0, 0).addMSecs(duration_ms).toString("H:mm:ss");
        ;
    }

    Q_INVOKABLE QVariantMap resolvePreviewSource(qint64 draft_block_id) {
        DraftBlock block = draft_block_list->blockAt(draft_block_id);

        QString file_path;
        if (block.kind == "memo") {
            file_path = PathUtils::resolvePathFromVideoId(block.source_path, m_setting->archiveDir(), "video");
        } else {
            file_path = block.source_path;
        }

        return QVariantMap{
            {"source", QUrl::fromLocalFile(file_path)},
            {"start", block.start_ms},
            {"end", block.end_ms}
        };
    }

    Q_INVOKABLE bool exportDraft() {
        const auto timeline =
            DraftExport::buildTimeline(draft_block_list->blocks(), m_setting->archiveDir());

        const QDir export_dir(PathUtils::appLocalDataDir().filePath("exports"));
        if (!export_dir.exists() && !QDir().mkpath(export_dir.absolutePath())) {
            qWarning() << "export: failed to create export dir" << export_dir.absolutePath();
            emit exportDraftFailed();
            return false;
        }

        QString safe_title = m_title;
        safe_title.replace('/', '_');
        safe_title.replace('\\', '_');
        if (safe_title.trimmed().isEmpty()) {
            safe_title = m_draft_id;
        }

        const QString xml_path = export_dir.filePath(safe_title + ".xml");
        const QString srt_path = export_dir.filePath(safe_title + ".srt");

        const bool xml_ok =
            DraftExport::writeTextFile(xml_path, DraftExport::buildXml(timeline, safe_title));
        const bool srt_ok = DraftExport::writeTextFile(srt_path, DraftExport::buildSrt(timeline));

        if (!xml_ok || !srt_ok) {
            emit exportDraftFailed();
            return false;
        }

        emit exportDraftFinished(export_dir.absolutePath());
        return true;
    }

    Q_SIGNAL void pendingSeTargetIdChanged();

    DraftBlockListModel* draft_block_list = nullptr;
    SrtCueListModel* srt_cue_list = nullptr;

    ProjectListModel* project_list = nullptr;
    MemoBlockListModel* memo_block_list = nullptr;

  private:
    [[nodiscard]] QString generateDraftId() const {
        static const QString chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

        auto draft_id_list = m_db->getAllDraftId();

        QString id;
        do {
            id.clear();
            for (int i = 0; i < 5; ++i) {
                id.append(chars.at(QRandomGenerator::global()->bounded(chars.length())));
            }
        } while (std::find(draft_id_list.begin(), draft_id_list.end(), id) != draft_id_list.end());

        return id;
    }

    qint32 getVideoDurationMs(const QString& filePath) {
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

        QString output = process.readAllStandardOutput().trimmed();
        bool ok = false;
        double seconds = output.toDouble(&ok);

        if (!ok) {
            return -1;
        }

        return static_cast<qint32>(seconds * 1000);
    }

    DatabaseManager* m_db;
    const SettingModel* m_setting;

    QString m_draft_id = "";
    QString m_title = "";
    QString m_concept = "";

    qint64 m_pending_se_target_id = -1;

    PreviewExporter m_exporter;

    std::vector<MemoProject> m_selected_projects;

    std::vector<SrtCue> m_srt_cues;
    QString m_srt_video_id;

  signals:
    void previewReady(const QString& path);
    void exportDraftFinished(const QString& exportDirPath);
    void exportDraftFailed();
};
