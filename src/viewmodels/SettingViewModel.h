#pragma once

#include "SettingModel.h"

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

class SettingViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created only from C++")
  public:
    explicit SettingViewModel(SettingModel& setting, QObject* parent = nullptr) : QObject(parent) {
        m_setting = &setting;
    }

    Q_INVOKABLE bool setArchiveDir(const QString& path) {
        auto result = m_setting->setArchiveDir(path);
        if (result) {
            qDebug() << "Invalid Value:" << result.value();
            return false;
        }

        return true;
    } // TODO: Response to UI when result is not nullopt

  private:
    SettingModel* m_setting;
};
