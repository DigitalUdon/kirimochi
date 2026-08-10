#pragma once

#include "PathUtils.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QString>

struct Setting {
    QDir archive_dir;
};

class SettingModel {
  public:
    SettingModel()
        : SettingModel(PathUtils::appLocalDataDir().filePath("settings.ini"),
                       PathUtils::appLocalDataDir()) {
    }

    SettingModel(const QString& ini_path, const QDir& default_archive_dir)
        : m_settings_object(ini_path, QSettings::IniFormat) {
        auto settingsFilePath = m_settings_object.fileName();

        if (!QFileInfo::exists(settingsFilePath)) {
            m_settings_object.beginGroup("Paths");
            m_settings_object.setValue("ArchiveDir", default_archive_dir.absolutePath());
            m_settings_object.endGroup();
        }

        if (auto message = setArchiveDir(m_settings_object.value("Paths/ArchiveDir").toString())) {
            m_invalid_value_messages.push_back(*message);
        }
    }

    std::optional<QString> setArchiveDir(const QString& path) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists() && fileInfo.isDir()) {
            m_setting.archive_dir = QDir(path);

            m_settings_object.beginGroup("Paths");
            m_settings_object.setValue("ArchiveDir", path);
            m_settings_object.endGroup();

            return std::nullopt;
        }

        return QString("Archive directory does not exist. Value unchanged.");
    }

    [[nodiscard]] bool hasInvalidValue() const {
        return !m_invalid_value_messages.empty();
    }

    [[nodiscard]] const std::vector<QString>& invalidValueMessages() const {
        return m_invalid_value_messages;
    }

    [[nodiscard]] QDir archiveDir() const {
        return m_setting.archive_dir;
    }

  private:
    QSettings m_settings_object;
    Setting m_setting;
    std::vector<QString> m_invalid_value_messages;
};
