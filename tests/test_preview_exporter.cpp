#include "PreviewExporter.h"
#include "viewmodels/models/DraftBlockListModel.h"

#include <QTest>
#include <QTemporaryDir>
#include <QDir>

class TestPreviewExporter: public QObject
{
    Q_OBJECT
  private slots:
    void buildPlan_skipsInvalidMemoAndImageBlocks() {
        // Arrange
        QVector<DraftBlock> blocks;

        DraftBlock valid_video;
        valid_video.kind = "video";
        valid_video.source_path = "/path/to/video.mp4";
        valid_video.start_ms = 0;
        valid_video.end_ms = 2000;
        blocks.append(valid_video);

        DraftBlock invalid_memo;
        invalid_memo.kind = "memo";
        invalid_memo.source_path = "";
        invalid_memo.start_ms = 0;
        invalid_memo.end_ms = 1000;
        blocks.append(invalid_memo);

        DraftBlock invalid_image;
        invalid_image.kind = "image";
        invalid_image.source_path = "/path/to/image.png";
        invalid_image.duration = 0;
        blocks.append(invalid_image);

        QTemporaryDir archive_dir;

        // Act
        auto plan = PreviewRender::buildPlan(blocks, QDir(archive_dir.path()));

        // Assert
        QCOMPARE(plan.segments.size(), 1);
        QCOMPARE(plan.segments.front().filePath, QStringLiteral("/path/to/video.mp4"));
    }
};

QTEST_MAIN(TestPreviewExporter)
#include "test_preview_exporter.moc"
