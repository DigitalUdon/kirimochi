#pragma once

#include <QAbstractListModel>
#include <QTime>
#include <QtQmlIntegration/qqmlintegration.h>

struct MemoBlock {
    qint64 memo_id;
    qint32 start_ms;
    qint32 end_ms;
    QString label;
    QString body;

    bool operator<(const MemoBlock& another) const {
        return end_ms < another.end_ms;
    }
};

class MemoBlockListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
  public:
    enum Roles { MemoId = Qt::UserRole + 1, Start, End, Label, Body };

    explicit MemoBlockListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
        return static_cast<int>(m_memo_block_list.size());
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(m_memo_block_list.size())) {
            return {};
        }

        const MemoBlock& memo_block = m_memo_block_list[index.row()];

        switch (role) {
        case MemoId:
            return memo_block.memo_id;
        case Start:
            return QTime(0, 0).addMSecs(memo_block.start_ms).toString("H:mm:ss");
        case End:
            return QTime(0, 0).addMSecs(memo_block.end_ms).toString("H:mm:ss");
        case Label:
            return memo_block.label;
        case Body:
            return memo_block.body;
        default:
            return {};
        }
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override {
        return {
            {MemoId, "memoId"}, {Start, "start"}, {End, "end"}, {Label, "label"}, {Body, "body"}};
    }

    void setMemoBlocks(const std::vector<MemoBlock>& memos) {
        beginResetModel();
        m_memo_block_list = memos;
        endResetModel();
    }

    void add(qint64 memo_id, qint32 start_ms, qint32 end_ms, const QString& label) {
        const int row = rowCount(QModelIndex());

        beginInsertRows(QModelIndex(), row, row);
        m_memo_block_list.push_back({memo_id, start_ms, end_ms, label, ""});
        endInsertRows();
    }

    bool updateStart(qint64 memo_id, qint32 ms) {
        int row = -1;
        for (int i = 0; i < m_memo_block_list.size(); ++i) {
            if (m_memo_block_list[i].memo_id == memo_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return false;
        }

        if (ms >= m_memo_block_list[row].end_ms) {
            return false;
        }

        m_memo_block_list[row].start_ms = ms;

        QModelIndex idx = createIndex(row, 0);
        emit dataChanged(idx, idx, {Start});

        return true;
    }

    bool updateEnd(qint64 memo_id, qint32 ms) {
        int row = -1;
        for (int i = 0; i < m_memo_block_list.size(); ++i) {
            if (m_memo_block_list[i].memo_id == memo_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return false;
        }

        if (ms <= m_memo_block_list[row].start_ms) {
            return false;
        }

        m_memo_block_list[row].end_ms = ms;

        QModelIndex idx = createIndex(row, 0);
        emit dataChanged(idx, idx, {End});

        return true;
    }

    void remove(qint64 memo_id) {
        int row = -1;
        for (int i = 0; i < m_memo_block_list.size(); ++i) {
            if (m_memo_block_list[i].memo_id == memo_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return;
        }

        beginRemoveRows(QModelIndex(), row, row);

        m_memo_block_list.erase(m_memo_block_list.begin() + row);

        endRemoveRows();
    }

  private:
    std::vector<MemoBlock> m_memo_block_list;
};
