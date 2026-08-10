#pragma once

#include "DatabaseManager.h"
#include "PathUtils.h"
#include "SettingModel.h"
#include "viewmodels/models/MemoBlockListModel.h"

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class ArchivePlayerViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created only from C++")
    Q_PROPERTY(MemoBlockListModel* memo_block_list MEMBER memo_block_list CONSTANT)
  public:
    explicit ArchivePlayerViewModel(DatabaseManager& db,
                                    const SettingModel& setting,
                                    const QString& video_id,
                                    QObject* parent = nullptr)
        : QObject(parent) {
        m_db = &db;
        m_setting = &setting;

        m_video_id = video_id;
        m_video_path =
            PathUtils::resolvePathFromVideoId(m_video_id, m_setting->archiveDir(), "video");

        m_chat_volume_data_path =
            PathUtils::resolvePathFromVideoId(m_video_id, m_setting->archiveDir(), "chat");

        QFile file(PathUtils::resolvePathFromVideoId(m_video_id, m_setting->archiveDir(), "title"));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_title = QTextStream(&file).readLine();
        }
        if (m_title.isEmpty()) {
            m_title = QStringLiteral("New memo");
        }

        memo_block_list = new MemoBlockListModel(this);

        auto memo_project = m_db->getMemoProject(m_video_id);
        if (memo_project.video_id == "") {
            m_db->insertMemoProject(m_video_id, m_title);
        } else {
            memo_block_list->setMemoBlocks(m_db->getMemos(m_video_id));
            m_media_position = memo_project.last_position_ms;
        }
    }

    // Settings
    Q_INVOKABLE QString title() {
        return m_title;
    }

    Q_INVOKABLE QString videoPath() {
        return QUrl::fromLocalFile(m_video_path).toString();
    }

    Q_INVOKABLE QString chatVolumeDataPath() {
        return m_chat_volume_data_path;
    }

    // Media Duration/Position
    Q_INVOKABLE QString mediaDurationAsString() {
        return QTime::fromMSecsSinceStartOfDay(m_media_duration).toString("hh:mm:ss");
    }

    Q_INVOKABLE void setMediaDuration(qint32 value) {
        m_media_duration = value;
    }

    Q_INVOKABLE qint32 mediaPositionAsMs() {
        return m_media_position;
    }

    Q_INVOKABLE void setMediaPosition(qint32 value) {
        m_media_position = value;
        m_db->updateMemoProjectLastPosition(m_video_id, value);
    }

    // Memo
    Q_INVOKABLE void addMemoBlock(qint32 end_ms, const QString& label) {
        qint32 start_ms = end_ms - 60000;
        if (start_ms < 0) {
            start_ms = 0;
        }

        qint64 memo_id = m_db->insertMemo(m_video_id, start_ms, end_ms, label);
        memo_block_list->add(memo_id, start_ms, end_ms, label);
    }

    Q_INVOKABLE void updateMemoComment(qint64 memo_id, const QString& body) {
        m_db->updateMemoComment(memo_id, body);
    }

    Q_INVOKABLE void updateMemoStart(qint64 memo_id, qint32 ms) {
        if (memo_block_list->updateStart(memo_id, ms)) {
            m_db->updateMemoStart(memo_id, ms);
        }
    }

    Q_INVOKABLE void updateMemoEnd(qint64 memo_id, qint32 ms) {
        if (memo_block_list->updateEnd(memo_id, ms)) {
            m_db->updateMemoEnd(memo_id, ms);
        }
    }

    Q_INVOKABLE void deleteMemoBlock(qint64 memo_id) {
        memo_block_list->remove(memo_id);
        m_db->deleteMemo(memo_id);
    }

    MemoBlockListModel* memo_block_list = nullptr;

  private:
    DatabaseManager* m_db;
    const SettingModel* m_setting;

    qint64 m_project_id = -1;

    QString m_video_id = "";
    QString m_video_path = "";
    QString m_chat_volume_data_path = "";

    QString m_title = "";

    qint32 m_media_duration = 0;
    qint32 m_media_position = 0;
};
