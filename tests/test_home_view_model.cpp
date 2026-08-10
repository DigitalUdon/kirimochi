#include "viewmodels/HomeViewModel.h"

#include "DatabaseManager.h"
#include "SettingModel.h"

#include <QTest>
#include <QTemporaryDir>

class TestHomeViewModel: public QObject
{
    Q_OBJECT
  private slots:
    void deleteProject_removesFromListModelAndDatabase() {
        DatabaseManager db;
        QVERIFY(db.init(true));

        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());
        SettingModel setting(temp_dir.filePath("settings.ini"), QDir(temp_dir.path()));

        // Arrange:
        db.insertMemoProject("test_video_id", "test_title");
        db.insertDraftProject("test_draft_id", "test_draft_title", "test_concept");

        HomeViewModel view_model(db, setting);

        QCOMPARE(view_model.project_list->rowCount(QModelIndex()), 2);

        // Act:
        view_model.updateProject("test_video_id", "Memo", "updated_title");

        // Assert:
        auto memo_project_after_update = db.getMemoProject("test_video_id");
        QCOMPARE(memo_project_after_update.video_title, QString("updated_title"));
        QCOMPARE(view_model.project_list->rowCount(QModelIndex()), 2);

        // Act:
        view_model.deleteProject("Memo", "test_video_id");

        // Assert:
        QCOMPARE(view_model.project_list->rowCount(QModelIndex()), 1);

        auto memo_project_after_delete = db.getMemoProject("test_video_id");
        QCOMPARE(memo_project_after_delete.video_id, QString());
    }
};

QTEST_MAIN(TestHomeViewModel)
#include "test_home_view_model.moc"
