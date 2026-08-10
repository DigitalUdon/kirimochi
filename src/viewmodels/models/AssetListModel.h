#pragma once

#include <QAbstractListModel>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

struct Asset {
    qint64 asset_id;
    QString name;
    QString kind;
    QString file_path;
    qint32 duration_ms = 0;
    qint32 width = 0;
    qint32 height = 0;
    QString provider = QString();
    QString tags = QString();
    qint64 updated_at;

    bool operator<(const Asset& another) const {
        return name < another.name;
    }

    bool operator>(const Asset& another) const {
        return name > another.name;
    }
};

class AssetListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
  public:
    enum Roles {
        AssetId = Qt::UserRole + 1,
        Name,
        Kind,
        FilePath,
        DurationMs,
        Width,
        Height,
        Provider,
        Tags
    };

    explicit AssetListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
        return static_cast<int>(asset_list.size());
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(asset_list.size())) {
            return {};
        }

        const Asset& asset = asset_list[index.row()];

        switch (role) {
        case AssetId:
            return asset.asset_id;
        case Name:
            return asset.name;
        case Kind:
            return asset.kind;
        case FilePath:
            return QUrl::fromLocalFile(asset.file_path).toString();
        case DurationMs:
            return asset.duration_ms;
        case Width:
            return asset.width;
        case Height:
            return asset.height;
        case Provider:
            return asset.provider;
        case Tags:
            return asset.tags;
        default:
            return {};
        }
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override {
        return {{AssetId, "assetId"},
                {Name, "name"},
                {Kind, "kind"},
                {FilePath, "filePath"},
                {DurationMs, "durationMs"},
                {Width, "width"},
                {Height, "height"},
                {Provider, "provider"},
                {Tags, "tags"}};
    }

    void setAssets(const std::vector<Asset>& assets) {
        beginResetModel();
        asset_list = assets;
        endResetModel();
    }

    void remove(qint64 asset_id) {
        int row = -1;
        for (int i = 0; i < asset_list.size(); ++i) {
            if (asset_list[i].asset_id == asset_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return;
        }

        beginRemoveRows(QModelIndex(), row, row);

        asset_list.erase(asset_list.begin() + row);

        endRemoveRows();
    }

  private:
    std::vector<Asset> asset_list;
};
