#pragma once

#include "DatabaseManager.h"
#include "SettingModel.h"
#include "viewmodels/ArchivePlayerViewModel.h"
#include "viewmodels/AssetLibraryViewModel.h"
#include "viewmodels/DraftEditorViewModel.h"
#include "viewmodels/IdeaLibraryViewModel.h"
#include "viewmodels/HomeViewModel.h"
#include "viewmodels/SettingViewModel.h"

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include <qdir.h>
#include <qtmetamacros.h>

class AppNavigator : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
  public:
    AppNavigator(QObject* parent = nullptr) : QObject(parent), m_db() {
        if (!m_db.init()) {
            m_has_db_error = true;
        }

        if (m_setting.hasInvalidValue()) {
            m_invalid_value_messages = m_setting.invalidValueMessages();
        }
    }

    AppNavigator* create(QQmlEngine*, QJSEngine*) {
        return new AppNavigator();
    }

    // Each page transition
    Q_INVOKABLE void goToHomePage() {
        if (!m_homeViewModel) {
            m_homeViewModel = new HomeViewModel(m_db, m_setting, this);
        }
        emit navigateToHome(m_homeViewModel);
    }

    Q_INVOKABLE void goToSettingDialog() {
        if (!m_settingViewModel) {
            m_settingViewModel = new SettingViewModel(m_setting, this);
        }
        emit navigateToSetting(m_settingViewModel);
    }

    Q_INVOKABLE void goToArchivePlayerPage(const QString& video_id) {
        if (!m_archivePlayerViewModel) {
            m_archivePlayerViewModel = new ArchivePlayerViewModel(m_db, m_setting, video_id, this);
        }
        emit navigateToArchivePlayer(m_archivePlayerViewModel);
    }

    Q_INVOKABLE void goToDraftEditorPage(const QString& title,
                                         const QString& concept,
                                         const QString& draft_id = QString()) {
        if (!m_draftEditorViewModel) {
            m_draftEditorViewModel =
                new DraftEditorViewModel(m_db, m_setting, title, concept, draft_id, this);
        }

        if (!m_assetLibraryViewModel) {
            m_assetLibraryViewModel = new AssetLibraryViewModel(m_db, m_setting, this);
        }

        if (!m_ideaLibraryViewModel) {
            m_ideaLibraryViewModel = new IdeaLibraryViewModel(m_db, m_setting, this);
        }

        emit navigateToDraftEditor(
            m_draftEditorViewModel, m_assetLibraryViewModel, m_ideaLibraryViewModel);
    }

    Q_INVOKABLE void goToLibraryPage() {
        if (!m_assetLibraryViewModel) {
            m_assetLibraryViewModel = new AssetLibraryViewModel(m_db, m_setting, this);
        }

        if (!m_ideaLibraryViewModel) {
            m_ideaLibraryViewModel = new IdeaLibraryViewModel(m_db, m_setting, this);
        }

        emit navigateToLibrary(m_assetLibraryViewModel, m_ideaLibraryViewModel);
    }

    // Error catch
    Q_INVOKABLE [[nodiscard]] std::vector<QString> invalidValueMessages() const {
        return m_invalid_value_messages;
    }

    Q_INVOKABLE [[nodiscard]] bool hasInvalidValue() const {
        return !m_invalid_value_messages.empty();
    }

    Q_INVOKABLE [[nodiscard]] bool hasDBError() const {
        return m_has_db_error;
    }

  signals:
    void navigateToHome(QObject* viewModel);
    void navigateToSetting(QObject* viewModel);
    void navigateToArchivePlayer(QObject* viewModel);
    void navigateToDraftEditor(QObject* draftEditorViewModel,
                               QObject* assetLibraryViewModel,
                               QObject* ideaLibraryViewModel);
    void navigateToLibrary(QObject* assetLibraryViewModel, QObject* ideaLibraryViewModel);

  private:
    DatabaseManager m_db;
    SettingModel m_setting;

    HomeViewModel* m_homeViewModel = nullptr;
    SettingViewModel* m_settingViewModel = nullptr;
    ArchivePlayerViewModel* m_archivePlayerViewModel = nullptr;
    DraftEditorViewModel* m_draftEditorViewModel = nullptr;
    AssetLibraryViewModel* m_assetLibraryViewModel = nullptr;
    IdeaLibraryViewModel* m_ideaLibraryViewModel = nullptr;

    bool m_has_db_error = false;
    std::vector<QString> m_invalid_value_messages;
};
