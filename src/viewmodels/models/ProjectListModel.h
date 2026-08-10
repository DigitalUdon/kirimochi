#pragma once

#include <QAbstractListModel>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

struct Project {
    QString project_id;
    QString kind;
    QString title;
    QString thumbnail_path;
    QString status;
    qint64 updated_at;

    bool operator<(const Project& another) const {
        return updated_at < another.updated_at;
    }

    bool operator>(const Project& another) const {
        return updated_at > another.updated_at;
    }
};

class ProjectListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
  public:
    enum Roles { ProjectId = Qt::UserRole + 1, Kind, Title, ThumbnailPath, Status };

    explicit ProjectListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
        return static_cast<int>(project_list.size());
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(project_list.size())) {
            return {};
        }

        const Project& project = project_list[index.row()];

        switch (role) {
        case ProjectId:
            return project.project_id;
        case Kind:
            return project.kind;
        case Title:
            return project.title;
        case ThumbnailPath:
            return QUrl::fromLocalFile(project.thumbnail_path).toString();
        case Status:
            return project.status;
        default:
            return {};
        }
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override {
        return {{ProjectId, "projectId"},
                {Kind, "kind"},
                {Title, "title"},
                {ThumbnailPath, "thumbnailPath"},
                {Status, "status"}};
    }

    void setProjects(const std::vector<Project>& projects) {
        beginResetModel();
        project_list = projects;
        endResetModel();
    }

    void remove(const QString& project_id) {
        int row = -1;
        for (int i = 0; i < project_list.size(); ++i) {
            if (project_list[i].project_id == project_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return;
        }

        beginRemoveRows(QModelIndex(), row, row);

        project_list.erase(project_list.begin() + row);

        endRemoveRows();
    }

  private:
    std::vector<Project> project_list;
};
