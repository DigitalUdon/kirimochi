#pragma once

#include "DatabaseManager.h"
#include "SettingModel.h"
#include "viewmodels/models/IdeaListModel.h"

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>
#include <qtmetamacros.h>

class IdeaLibraryViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created only from C++")
    Q_PROPERTY(IdeaListModel* idea_list MEMBER idea_list CONSTANT)
  public:
    explicit IdeaLibraryViewModel(DatabaseManager& db,
                                         const SettingModel& setting,
                                         QObject* parent = nullptr)
        : QObject(parent) {
        m_db = &db;
        m_setting = &setting;
        idea_list = new IdeaListModel(this);

        loadIdeas();
    }

    Q_INVOKABLE void registerIdea(const QString& name, const QString& description, const QString& tags) {
        m_db->insertIdea(name, description, tags);

        loadIdeas();
    }

    Q_INVOKABLE QVariantMap getIdea(qint64 idea_id) {
        Idea idea = m_db->getIdea(idea_id);

        QVariantMap map;
        map["idea_id"] = idea.idea_id;
        map["name"] = idea.name;
        map["description"] = idea.description;
        map["tags"] = idea.tags;
        map["updated_at"] = idea.updated_at;

        return map;
    }

    Q_INVOKABLE void updateIdea(qint64 idea_id,
                                 const QString& name,
                                 const QString& description,
                                 const QString& tags) {
        m_db->updateIdea(idea_id, name, description, tags);

        loadIdeas();
    }

    Q_INVOKABLE void deleteIdea(qint64 idea_id) {
        idea_list->remove(idea_id);
        m_db->deleteIdea(idea_id);
    }

    IdeaListModel* idea_list = nullptr;

  private:
    void loadIdeas() {
        idea_list->setIdeas(m_db->getAllIdeas());
    }

    DatabaseManager* m_db;
    const SettingModel* m_setting;
};
