#pragma once

#include <QAbstractListModel>
#include <QTime>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

struct DraftBlock {
    qint64 draft_block_id;
    QString kind;
    QString source_path;
    qint32 duration;
    qint32 start_ms;
    qint32 end_ms;
    QString label;
    QString body;
    QString se_path;
    qint32 se_offset_ms;
};

class DraftBlockListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ANONYMOUS
  public:
    enum Roles {
        DraftBlockId = Qt::UserRole + 1,
        Kind,
        SourcePath,
        Duration,
        Start,
        End,
        Label,
        Body,
        SePath,
        SeOffsetMs
    };

    explicit DraftBlockListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {
    }

    [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
        return static_cast<int>(m_draft_block_list.size());
    }

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= static_cast<int>(m_draft_block_list.size())) {
            return {};
        }

        const DraftBlock& draft_block = m_draft_block_list[index.row()];

        switch (role) {
        case DraftBlockId:
            return draft_block.draft_block_id;
        case Kind:
            return draft_block.kind;
        case SourcePath:
            return QUrl::fromLocalFile(draft_block.source_path).toString();
        case Duration:
            return QTime(0, 0).addMSecs(draft_block.duration).toString("H:mm:ss");
        case Start:
            return QTime(0, 0).addMSecs(draft_block.start_ms).toString("H:mm:ss");
        case End:
            return QTime(0, 0).addMSecs(draft_block.end_ms).toString("H:mm:ss");
        case Label:
            return draft_block.label;
        case Body:
            return draft_block.body;
        case SePath:
            return draft_block.se_path;
        case SeOffsetMs:
            return draft_block.se_offset_ms;
        default:
            return {};
        }
    }

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override {
        return {{DraftBlockId, "draftBlockId"},
                {Kind, "kind"},
                {SourcePath, "sourcePath"},
                {Duration, "duration"},
                {Start, "start"},
                {End, "end"},
                {Label, "label"},
                {Body, "body"},
                {SePath, "sePath"},
                {SeOffsetMs, "seOffsetMs"}};
    }

    void setDraftBlocks(const std::vector<DraftBlock>& memos) {
        beginResetModel();
        m_draft_block_list = memos;
        endResetModel();
    }

    void add(qint64 draft_block_id,
             const QString& kind,
             const QString& source_path,
             qint32 duration,
             qint32 start_ms,
             qint32 end_ms,
             const QString& label,
             const QString& body,
             const QString& se_path,
             qint32 se_offset_ms) {
        const int row = rowCount(QModelIndex());

        beginInsertRows(QModelIndex(), row, row);
        m_draft_block_list.push_back({draft_block_id,
                                      kind,
                                      source_path,
                                      duration,
                                      start_ms,
                                      end_ms,
                                      label,
                                      body,
                                      se_path,
                                      se_offset_ms});
        endInsertRows();
    }

    void remove(qint64 draft_block_id) {
        int row = -1;
        for (int i = 0; i < m_draft_block_list.size(); ++i) {
            if (m_draft_block_list[i].draft_block_id == draft_block_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return;
        }

        beginRemoveRows(QModelIndex(), row, row);

        m_draft_block_list.erase(m_draft_block_list.begin() + row);

        endRemoveRows();
    }

    void insertAfter(qint64 after_draft_block_id, const DraftBlock& new_block) {
        int row = -1;
        for (int i = 0; i < m_draft_block_list.size(); ++i) {
            if (m_draft_block_list[i].draft_block_id == after_draft_block_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return;
        }

        const int insert_row = row + 1;
        beginInsertRows(QModelIndex(), insert_row, insert_row);
        m_draft_block_list.insert(m_draft_block_list.begin() + insert_row, new_block);
        endInsertRows();
    }

    void move(qint32 from, qint32 to) {
        if (from == to)
            return;
        if (from < 0 || from >= static_cast<qint32>(m_draft_block_list.size()))
            return;
        if (to < 0 || to >= static_cast<qint32>(m_draft_block_list.size()))
            return;

        qint32 destination = (to > from) ? to + 1 : to;
        beginMoveRows(QModelIndex(), from, from, QModelIndex(), destination);

        if (from < to)
            std::rotate(m_draft_block_list.begin() + from,
                        m_draft_block_list.begin() + from + 1,
                        m_draft_block_list.begin() + to + 1);
        else
            std::rotate(m_draft_block_list.begin() + to,
                        m_draft_block_list.begin() + from,
                        m_draft_block_list.begin() + from + 1);

        endMoveRows();
    }

    std::vector<qint64> getOrderedIds() {
        std::vector<qint64> ordered_ids;
        ordered_ids.reserve(m_draft_block_list.size());
        for (const auto& item : m_draft_block_list) {
            ordered_ids.push_back(item.draft_block_id);
        }

        return ordered_ids;
    }

    DraftBlock blockAt(qint64 draft_block_id) {
        int row = -1;
        for (int i = 0; i < m_draft_block_list.size(); ++i) {
            if (m_draft_block_list[i].draft_block_id == draft_block_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return DraftBlock{};
        }

        return m_draft_block_list[row];
    }

    QVector<DraftBlock> blocks() {
        QVector<DraftBlock> qv(m_draft_block_list.begin(), m_draft_block_list.end());
        return qv;
    }

    bool attachSe(qint64 draft_block_id, const QString& file_path) {
        int row = -1;
        for (int i = 0; i < m_draft_block_list.size(); ++i) {
            if (m_draft_block_list[i].draft_block_id == draft_block_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return false;
        }

        m_draft_block_list[row].se_path = file_path;

        QModelIndex idx = createIndex(row, 0);
        emit dataChanged(idx, idx, {SePath});

        return true;
    }

    bool removeSe(qint64 draft_block_id) {
        int row = -1;
        for (int i = 0; i < m_draft_block_list.size(); ++i) {
            if (m_draft_block_list[i].draft_block_id == draft_block_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return false;
        }

        m_draft_block_list[row].se_path = "";

        QModelIndex idx = createIndex(row, 0);
        emit dataChanged(idx, idx, {SePath});

        return true;
    }

    bool updateDuration(qint64 draft_block_id, qint32 duration_ms) {
        int row = -1;
        for (int i = 0; i < m_draft_block_list.size(); ++i) {
            if (m_draft_block_list[i].draft_block_id == draft_block_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return false;
        }

        m_draft_block_list[row].duration = duration_ms;

        QModelIndex idx = createIndex(row, 0);
        emit dataChanged(idx, idx, {Duration});

        return true;
    }

    bool updateRange(qint64 draft_block_id, qint32 start_ms, qint32 end_ms) {
        int row = -1;
        for (int i = 0; i < m_draft_block_list.size(); ++i) {
            if (m_draft_block_list[i].draft_block_id == draft_block_id) {
                row = i;
                break;
            }
        }

        if (row == -1) {
            return false;
        }

        m_draft_block_list[row].start_ms = start_ms;
        m_draft_block_list[row].end_ms = end_ms;

        QModelIndex idx = createIndex(row, 0);
        emit dataChanged(idx, idx, {Start, End});

        return true;
    }

  private:
    std::vector<DraftBlock> m_draft_block_list;
};
