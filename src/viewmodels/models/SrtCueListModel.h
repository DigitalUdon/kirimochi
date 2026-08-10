#pragma once

#include "SrtUtils.h"

#include <QAbstractListModel>
#include <QTime>
#include <QtQmlIntegration/qqmlintegration.h>

#include <vector>

class SrtCueListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
  public:
    enum Roles {
        Start = Qt::UserRole + 1,
        End,
        Text,
        Selected
    };

    struct SelectedRange {
        bool valid;
        qint32 start_ms;
        qint32 end_ms;
    };

    explicit SrtCueListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
        return static_cast<int>(m_items.size());
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(m_items.size())) {
            return {};
        }

        const Item& item = m_items[index.row()];

        switch (role) {
        case Start:
            return QTime(0, 0).addMSecs(item.cue.start_ms).toString("H:mm:ss");
        case End:
            return QTime(0, 0).addMSecs(item.cue.end_ms).toString("H:mm:ss");
        case Text:
            return item.cue.text;
        case Selected:
            return item.selected;
        default:
            return {};
        }
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override {
        return {{Start, "start"},
                {End, "end"},
                {Text, "text"},
                {Selected, "selected"}};
    }

    void setCues(const std::vector<SrtCue>& cues) {
        beginResetModel();
        m_items.clear();
        m_items.reserve(cues.size());
        for (const auto& cue : cues) {
            m_items.push_back({cue, false});
        }
        endResetModel();
    }

    Q_INVOKABLE void toggleSelect(int row) {
        if (row < 0 || row >= static_cast<int>(m_items.size())) {
            return;
        }

        Item& item = m_items[row];
        if (!item.selected && selectedCount() >= 2) {
            return;
        }

        item.selected = !item.selected;

        const QModelIndex idx = createIndex(row, 0);
        emit dataChanged(idx, idx, {Selected});
    }

    Q_INVOKABLE void reset() {
        beginResetModel();
        m_items.clear();
        endResetModel();
    }

    [[nodiscard]] SelectedRange getSelectedRange() const {
        qint32 start_ms = -1;
        qint32 end_ms = -1;

        for (const auto& item : m_items) {
            if (!item.selected) {
                continue;
            }
            if (start_ms < 0) {
                start_ms = item.cue.start_ms;
            }
            end_ms = item.cue.end_ms;
        }

        if (start_ms < 0) {
            return {false, 0, 0};
        }

        return {true, start_ms, end_ms};
    }

  private:
    struct Item {
        SrtCue cue;
        bool selected;
    };

    [[nodiscard]] int selectedCount() const {
        int count = 0;
        for (const auto& item : m_items) {
            if (item.selected) {
                ++count;
            }
        }
        return count;
    }

    std::vector<Item> m_items;
};
