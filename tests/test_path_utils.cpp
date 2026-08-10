#include "PathUtils.h"

#include <QTest>
#include <QTemporaryDir>
#include <QDir>

class TestPathUtils: public QObject
{
    Q_OBJECT
  private slots:
    void resolvePathFromVideoId_returnsEmptyForEmptyVideoId() {
        // Arrange
        QTemporaryDir archive_dir;
        QVERIFY(archive_dir.isValid());

        QDir dir(archive_dir.path());
        QVERIFY(dir.mkpath("some_video_id"));

        QFile video_file(archive_dir.filePath("some_video_id/video.mp4"));
        QVERIFY(video_file.open(QIODevice::WriteOnly));
        video_file.close();

        // Act
        const QString result = PathUtils::resolvePathFromVideoId(
            "", QDir(archive_dir.path()), "video");

        // Assert
        QCOMPARE(result, QString());
    }
};

QTEST_MAIN(TestPathUtils)
#include "test_path_utils.moc"
