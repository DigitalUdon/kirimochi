#include "SrtUtils.h"

#include <QTest>
#include <QTemporaryDir>

class TestSrtUtils : public QObject
{
    Q_OBJECT

    static QString writeSrt(const QTemporaryDir& dir, const QByteArray& content,
                            const QString& name = "test.srt") {
        const QString path = dir.filePath(name);
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(content);
        file.close();
        return path;
    }

  private slots:
    void parseTimecode_basic() {
        QCOMPARE(SrtUtils::parseTimecode("00:00:01,500"), 1500);
        QCOMPARE(SrtUtils::parseTimecode("01:02:03,456"), 3723456);
        QCOMPARE(SrtUtils::parseTimecode("00:00:00,000"), 0);
        QCOMPARE(SrtUtils::parseTimecode("00:00:01.500"), 1500);
    }

    void parseTimecode_invalid() {
        QCOMPARE(SrtUtils::parseTimecode(""), -1);
        QCOMPARE(SrtUtils::parseTimecode("00:00:01"), -1);
        QCOMPARE(SrtUtils::parseTimecode("00:01,500"), -1);
        QCOMPARE(SrtUtils::parseTimecode("aa:bb:cc,ddd"), -1);
    }

    void parseSrt_basic() {
        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());

        const QByteArray srt =
            "1\n"
            "00:00:01,000 --> 00:00:03,500\n"
            "こんにちは\n"
            "\n"
            "2\n"
            "00:00:05,000 --> 00:00:07,000\n"
            "二つ目のセリフ\n";

        const auto cues = SrtUtils::parseSrt(writeSrt(temp_dir, srt));

        QCOMPARE(static_cast<int>(cues.size()), 2);
        QCOMPARE(cues[0].index, 1);
        QCOMPARE(cues[0].start_ms, 1000);
        QCOMPARE(cues[0].end_ms, 3500);
        QCOMPARE(cues[0].text, QString("こんにちは"));
        QCOMPARE(cues[1].start_ms, 5000);
        QCOMPARE(cues[1].text, QString("二つ目のセリフ"));
    }

    void parseSrt_multilineText() {
        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());

        const QByteArray srt =
            "1\n"
            "00:00:01,000 --> 00:00:03,000\n"
            "一行目\n"
            "二行目\n";

        const auto cues = SrtUtils::parseSrt(writeSrt(temp_dir, srt));

        QCOMPARE(static_cast<int>(cues.size()), 1);
        QCOMPARE(cues[0].text, QString("一行目\n二行目"));
    }

    void parseSrt_crlfAndBom() {
        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());

        const QByteArray srt =
            "\xEF\xBB\xBF"
            "1\r\n"
            "00:00:01,000 --> 00:00:02,000\r\n"
            "CRLFのセリフ\r\n"
            "\r\n"
            "2\r\n"
            "00:00:03,000 --> 00:00:04,000\r\n"
            "続き\r\n";

        const auto cues = SrtUtils::parseSrt(writeSrt(temp_dir, srt));

        QCOMPARE(static_cast<int>(cues.size()), 2);
        QCOMPARE(cues[0].start_ms, 1000);
        QCOMPARE(cues[0].text, QString("CRLFのセリフ"));
    }

    void parseSrt_malformedBlockIsSkipped() {
        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());

        const QByteArray srt =
            "1\n"
            "00:00:01,000 --> 00:00:02,000\n"
            "正常1\n"
            "\n"
            "2\n"
            "壊れたタイムコード行\n"
            "巻き添えテキスト\n"
            "\n"
            "3\n"
            "00:00:05,000 --> 00:00:06,000\n"
            "正常2\n";

        const auto cues = SrtUtils::parseSrt(writeSrt(temp_dir, srt));

        QCOMPARE(static_cast<int>(cues.size()), 2);
        QCOMPARE(cues[0].text, QString("正常1"));
        QCOMPARE(cues[1].index, 3);
        QCOMPARE(cues[1].text, QString("正常2"));
    }

    void parseSrt_noIndexLineVariant() {
        QTemporaryDir temp_dir;
        QVERIFY(temp_dir.isValid());

        const QByteArray srt =
            "00:00:01,000 --> 00:00:02,000\n"
            "番号なし\n";

        const auto cues = SrtUtils::parseSrt(writeSrt(temp_dir, srt));

        QCOMPARE(static_cast<int>(cues.size()), 1);
        QCOMPARE(cues[0].index, 1);
        QCOMPARE(cues[0].text, QString("番号なし"));
    }

    void parseSrt_missingFileReturnsEmpty() {
        const auto cues = SrtUtils::parseSrt("/path/to/nowhere.srt");
        QCOMPARE(static_cast<int>(cues.size()), 0);
    }
};

QTEST_MAIN(TestSrtUtils)
#include "test_srt_utils.moc"
