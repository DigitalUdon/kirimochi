#pragma once

#include "DatabaseManager.h"
#include "SettingModel.h"
#include "viewmodels/models/ProjectListModel.h"

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>
#include <qhashfunctions.h>
#include <qlogging.h>
#include <qtmetamacros.h>

class HomeViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created only from C++")
    Q_PROPERTY(ProjectListModel* project_list MEMBER project_list CONSTANT)
  public:
    explicit HomeViewModel(DatabaseManager& db,
                           const SettingModel& setting,
                           QObject* parent = nullptr)
        : QObject(parent) {
        m_db = &db;
        m_setting = &setting;
        project_list = new ProjectListModel(this);

        loadProjects();
    }

    Q_INVOKABLE void updateProject(const QString& project_id, const QString& kind, const QString& title) {
        if (kind == "Memo") {
            m_db->updateMemoProject(project_id, title);
        } else if (kind == "Draft") {
            m_db->updateDraftProject(project_id, title);
        } else {
            qWarning() << "Invalid value: " + kind;
        }

        loadProjects();
    }

    Q_INVOKABLE void deleteProject(const QString& kind, const QString& project_id) {
        project_list->remove(project_id);

        if (kind == "Memo") {
            m_db->deleteMemoProject(project_id);
        } else if (kind == "Draft") {
            m_db->deleteDraftProject(project_id);
        } else {
            qWarning() << "Invalid value: " + kind;
        }
    }

    Q_INVOKABLE void loadProjects() {
        auto projects = m_db->getAllProjects();
        for (auto& project : projects) {
            project.thumbnail_path = PathUtils::resolvePathFromVideoId(
                project.project_id, m_setting->archiveDir(), "thumbnail");
        }
        project_list->setProjects(projects);
    }

    Q_INVOKABLE QUrl archiveDir() {
        return QUrl::fromLocalFile(m_setting->archiveDir().path());
    }

    ProjectListModel* project_list = nullptr;

  private:
    DatabaseManager* m_db;
    const SettingModel* m_setting;
};
