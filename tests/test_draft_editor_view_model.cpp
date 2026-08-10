#include "viewmodels/DraftEditorViewModel.h"
#include "viewmodels/ArchivePlayerViewModel.h"

#include "DatabaseManager.h"
#include "SettingModel.h"

#include <QTest>
#include <QTemporaryDir>

class TestDraftEditorViewModel: public QObject
{
    Q_OBJECT
  private slots:
    void addMemo_persistsStartEndAndSourcePath() {
        DatabaseManager db;
        QVERIFY(db.init(true));

        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());
        SettingModel setting(temp_dir.filePath("settings.ini"), QDir(temp_dir.path()));

        // Arrange
        const QString video_id = "test_video_id";
        ArchivePlayerViewModel archive_player_view_model(db, setting, video_id);
        archive_player_view_model.addMemoBlock(2000, "label");

        auto memos = db.getMemos(video_id);
        QCOMPARE(static_cast<int>(memos.size()), 1);
        const qint64 memo_id = memos.front().memo_id;

        // Arrange
        const QString draft_id = "test_draft_id";
        db.insertDraftProject(draft_id, "title", "concept");

        DraftEditorViewModel view_model(db, setting, "title", "concept", draft_id);

        // Act
        view_model.addMemo(memo_id, 1000, 3000, "label", "body");

        // Assert
        auto draft_items = db.getDraftItems(draft_id);
        QCOMPARE(static_cast<int>(draft_items.size()), 1);

        const auto& item = draft_items.front();
        QCOMPARE(item.kind, QStringLiteral("memo"));
        QCOMPARE(item.start_ms, 1000);
        QCOMPARE(item.end_ms, 3000);
        QCOMPARE(item.source_path, video_id);
    }
};

QTEST_MAIN(TestDraftEditorViewModel)
#include "test_draft_editor_view_model.moc"
