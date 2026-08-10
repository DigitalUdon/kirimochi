#include "viewmodels/IdeaLibraryViewModel.h"

#include "DatabaseManager.h"
#include "SettingModel.h"

#include <QTest>
#include <QTemporaryDir>

class TestIdeaLibraryViewModel: public QObject
{
    Q_OBJECT
  private slots:
    void initRegisterDelete_reflectsInListModelAndDatabase() {
        DatabaseManager db;
        QVERIFY(db.init(true));

        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());
        SettingModel setting(temp_dir.filePath("settings.ini"), QDir(temp_dir.path()));

        // Arrange:
        IdeaLibraryViewModel view_model(db, setting);
        QCOMPARE(view_model.idea_list->rowCount(QModelIndex()), 0);

        // Act:
        view_model.registerIdea("test_idea", "description", "tags");

        // Assert:
        QCOMPARE(view_model.idea_list->rowCount(QModelIndex()), 1);

        auto ideas_after_register = db.getAllIdeas();
        QCOMPARE(static_cast<int>(ideas_after_register.size()), 1);
        const qint64 idea_id = ideas_after_register.front().idea_id;

        // Act:
        view_model.updateIdea(idea_id, "test_idea_renamed", "description2", "tags2");

        // Assert:
        QCOMPARE(view_model.idea_list->rowCount(QModelIndex()), 1);

        auto idea_after_update = db.getIdea(idea_id);
        QCOMPARE(idea_after_update.name, QString("test_idea_renamed"));
        QCOMPARE(idea_after_update.description, QString("description2"));
        QCOMPARE(idea_after_update.tags, QString("tags2"));

        // Act:
        view_model.deleteIdea(idea_id);

        // Assert:
        QCOMPARE(view_model.idea_list->rowCount(QModelIndex()), 0);

        auto ideas_after_delete = db.getAllIdeas();
        QCOMPARE(static_cast<int>(ideas_after_delete.size()), 0);
    }
};

QTEST_MAIN(TestIdeaLibraryViewModel)
#include "test_idea_library_view_model.moc"
