#include "viewmodels/ArchivePlayerViewModel.h"

#include "DatabaseManager.h"
#include "SettingModel.h"

#include <QTest>
#include <QTemporaryDir>

class TestArchivePlayerViewModel: public QObject
{
    Q_OBJECT
  private slots:
    void memoBlockLifecycle_addUpdateDelete() {
        DatabaseManager db;
        QVERIFY(db.init(true));

        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());
        SettingModel setting(temp_dir.filePath("settings.ini"), QDir(temp_dir.path()));

        const QString video_id = "test_video_id";
        ArchivePlayerViewModel view_model(db, setting, video_id);

        // --- Add ---
        view_model.addMemoBlock(2000, "label");

        auto memos_after_add = db.getMemos(video_id);
        QCOMPARE(static_cast<int>(memos_after_add.size()), 1);
        const qint64 memo_id = memos_after_add.front().memo_id;

        QCOMPARE(view_model.memo_block_list->rowCount(QModelIndex()), 1);

        // --- Update (Comment / Start / End) ---
        view_model.updateMemoComment(memo_id, "comment");
        view_model.updateMemoStart(memo_id, 1000);
        view_model.updateMemoEnd(memo_id, 3000);

        auto memos_after_update = db.getMemos(video_id);
        QCOMPARE(static_cast<int>(memos_after_update.size()), 1);
        const auto& updated = memos_after_update.front();
        QCOMPARE(updated.body, QString("comment"));
        QCOMPARE(updated.start_ms, 1000);
        QCOMPARE(updated.end_ms, 3000);

        const QModelIndex idx = view_model.memo_block_list->index(0);
        QCOMPARE(idx.data(MemoBlockListModel::Start).toString(), "0:00:01");
        QCOMPARE(idx.data(MemoBlockListModel::End).toString(), "0:00:03");

        // --- Delete ---
        view_model.deleteMemoBlock(memo_id);

        QCOMPARE(view_model.memo_block_list->rowCount(QModelIndex()), 0);

        auto memos_after_delete = db.getMemos(video_id);
        QCOMPARE(static_cast<int>(memos_after_delete.size()), 0);
    }
};

QTEST_MAIN(TestArchivePlayerViewModel)
#include "test_archive_player_view_model.moc"
