#pragma once

#include <QAbstractListModel>
#include <QtQmlIntegration/qqmlintegration.h>

struct Idea {
    qint64 idea_id;
    QString name;
    QString description;
    QString tags;
    qint64 updated_at;

    bool operator<(const Idea& another) const {
        return name < another.name;
    }

    bool operator>(const Idea& another) const {
        return name > another.name;
    }
};

class IdeaListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
  public:
    enum Roles { IdeaId = Qt::UserRole + 1, Name, Description, Tags };

    explicit IdeaListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
        return static_cast<int>(idea_list.size());
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(idea_list.size())) {
            return {};
        }

        const Idea& idea = idea_list[index.row()];

        switch (role) {
        case IdeaId:
            return idea.idea_id;
        case Name:
            return idea.name;
        case Description:
            return idea.description;
        case Tags:
            return idea.tags;
        default:
            return {};
        }
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override {
        return {{IdeaId, "ideaId"}, {Name, "name"}, {Description, "description"}, {Tags, "tags"}};
    }

    void setIdeas(const std::vector<Idea>& ideas) {
        beginResetModel();
        idea_list = ideas;
        endResetModel();
    }

    void remove(qint64 idea_id) {
        int row = -1;
        for (int i = 0; i < idea_list.size(); ++i) {
            if (idea_list[i].idea_id == idea_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return;
        }

        beginRemoveRows(QModelIndex(), row, row);

        idea_list.erase(idea_list.begin() + row);

        endRemoveRows();
    }

  private:
    std::vector<Idea> idea_list;
};
