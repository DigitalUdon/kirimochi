#pragma once

#include "DatabaseManager.h"
#include "SettingModel.h"
#include "viewmodels/models/AssetListModel.h"

#include <QObject>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>
#include <QProcess>
#include <qtmetamacros.h>

class AssetLibraryViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created only from C++")
    Q_PROPERTY(AssetListModel* asset_list MEMBER asset_list CONSTANT)
  public:
    explicit AssetLibraryViewModel(DatabaseManager& db,
                                   const SettingModel& setting,
                                   QObject* parent = nullptr)
        : QObject(parent) {
        m_db = &db;
        m_setting = &setting;
        asset_list = new AssetListModel(this);

        loadAssets();
    }

    Q_INVOKABLE void registerAsset(const QString& name,
                                   const QString& kind,
                                   const QString& file_path,
                                   qint32 duration_ms = 0,
                                   qint32 width = 0,
                                   qint32 height = 0,
                                   const QString& provider = QString(),
                                   const QString& tags = QString()) {
        const QString localPath = QUrl(file_path).toLocalFile();
        m_db->insertAsset(name, kind, localPath, duration_ms, width, height, provider, tags);

        loadAssets();
    }

    Q_INVOKABLE QVariantMap getAsset(qint64 asset_id) {
        Asset asset = m_db->getAsset(asset_id);

        QVariantMap map;
        map["asset_id"] = asset.asset_id;
        map["name"] = asset.name;
        map["kind"] = asset.kind;
        map["file_path"] = asset.file_path;
        map["duration_ms"] = asset.duration_ms;
        map["width"] = asset.width;
        map["height"] = asset.height;
        map["provider"] = asset.provider;
        map["tags"] = asset.tags;
        map["updated_at"] = asset.updated_at;

        return map;
    }

    Q_INVOKABLE void updateAsset(qint64 asset_id, 
                                const QString& name,
                                   const QString& kind,
                                   const QString& file_path,
                                   qint32 duration_ms,
                                   qint32 width,
                                   qint32 height,
                                   const QString& provider,
                                   const QString& tags) {
        m_db->updateAsset(asset_id, name, kind, file_path, duration_ms, width, height, provider, tags);
        loadAssets();
    }

    Q_INVOKABLE void deleteAsset(qint64 asset_id) {
        asset_list->remove(asset_id);
        m_db->deleteAsset(asset_id);
    }

    Q_INVOKABLE QVariantMap getMediaInfo(const QString& filePath, const QString& kind) {
        qDebug() << filePath << kind;
        QStringList entries;
        if (kind == "image") {
            entries << "stream=width,height";
        } else if (kind == "video") {
            entries << "format=duration" << "stream=width,height";
        } else {
            entries << "format=duration";
        }

        QProcess process;
        process.start("ffprobe",
                      {"-v",
                       "error",
                       "-show_entries",
                       entries.join(':'),
                       "-of",
                       "default=noprint_wrappers=1",
                       QUrl(filePath).toLocalFile()});

        process.waitForFinished();

        const QStringList lines =
            QString::fromUtf8(process.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
        qDebug() << lines;

        QVariantMap result;
        for (const QString& line : lines) {
            const int eq = line.indexOf('=');
            if (eq < 0) {
                continue;
            }
            const QString key = line.left(eq);
            const QString value = line.mid(eq + 1);

            if (key == "duration") {
                bool ok = false;
                const double seconds = value.toDouble(&ok);
                if (ok) {
                    result["duration_ms"] = static_cast<qint32>(seconds * 1000);
                }
            } else if (key == "width") {
                result["width"] = value.toInt();
            } else if (key == "height") {
                result["height"] = value.toInt();
            }
        }
        qDebug() << result;
        return result;
    }

    AssetListModel* asset_list = nullptr;

  private:
    void loadAssets() {
        asset_list->setAssets(m_db->getAllAssets());
    }

    DatabaseManager* m_db;
    const SettingModel* m_setting;
};
