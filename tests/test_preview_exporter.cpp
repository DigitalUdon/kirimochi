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

    void buildPlan_detectsMissingAudioStream() {
        QVector<DraftBlock> blocks;
        DraftBlock silent;
        silent.kind = "video";
        silent.source_path = QFINDTESTDATA("fixtures/no_audio.mp4");
        silent.end_ms = 1000;
        blocks.append(silent);

        QTemporaryDir archiveDir;
        auto plan = PreviewRender::buildPlan(blocks, QDir(archiveDir.path()));

        QCOMPARE(plan.segments.size(), 1);
        QVERIFY(!plan.segments.front().hasAudio);
    }

    void buildArgs_usesAnullsrcForVideoWithoutAudio() {
        PreviewRender::Plan plan;

        PreviewRender::Segment withAudio;
        withAudio.filePath = "/path/a.mp4";
        withAudio.endMs = 2000; withAudio.durationMs = 2000;
        withAudio.hasAudio = true;
        plan.segments.append(withAudio);

        PreviewRender::Segment noAudio;
        noAudio.filePath = "/path/b.mkv";
        noAudio.endMs = 3000; noAudio.durationMs = 3000;
        noAudio.hasAudio = false;
        plan.segments.append(noAudio);

        const auto args = PreviewRender::buildArgs(plan, "/tmp/out.mp4");
        const QString fc = args[args.indexOf("-filter_complex") + 1];

        QVERIFY(fc.contains("[0:a]atrim"));
        QVERIFY(!fc.contains("[1:a]atrim"));
        QVERIFY(fc.contains("anullsrc=r=48000:cl=stereo,atrim=end=3.000[a1]"));
    }
};

QTEST_MAIN(TestPreviewExporter)
#include "test_preview_exporter.moc"
