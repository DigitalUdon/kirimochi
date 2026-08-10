#include "viewmodels/AssetLibraryViewModel.h"

#include "DatabaseManager.h"
#include "SettingModel.h"

#include <QTest>
#include <QTemporaryDir>

class TestAssetLibraryViewModel: public QObject
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
        AssetLibraryViewModel view_model(db, setting);
        QCOMPARE(view_model.asset_list->rowCount(QModelIndex()), 0);

        // Act:
        const QString file_path = QUrl::fromLocalFile(temp_dir.filePath("test_se.wav")).toString();
        view_model.registerAsset("test_se", "se", file_path, 1000, 0, 0, "provider", "tag1,tag2");

        // Assert:
        QCOMPARE(view_model.asset_list->rowCount(QModelIndex()), 1);

        auto assets_after_register = db.getAllAssets();
        QCOMPARE(static_cast<int>(assets_after_register.size()), 1);
        const qint64 asset_id = assets_after_register.front().asset_id;

        // Act:
        view_model.updateAsset(asset_id, "test_se_renamed", "se", file_path, 2000, 0, 0, "provider2", "tag1,tag2,tag3");

        // Assert:
        QCOMPARE(view_model.asset_list->rowCount(QModelIndex()), 1);

        auto asset_after_update = db.getAsset(asset_id);
        QCOMPARE(asset_after_update.name, QString("test_se_renamed"));
        QCOMPARE(asset_after_update.duration_ms, 2000);
        QCOMPARE(asset_after_update.provider, QString("provider2"));
        QCOMPARE(asset_after_update.tags, QString("tag1,tag2,tag3"));

        // Act:
        view_model.deleteAsset(asset_id);

        // Assert:
        QCOMPARE(view_model.asset_list->rowCount(QModelIndex()), 0);

        auto assets_after_delete = db.getAllAssets();
        QCOMPARE(static_cast<int>(assets_after_delete.size()), 0);
    }
};

QTEST_MAIN(TestAssetLibraryViewModel)
#include "test_asset_library_view_model.moc"
