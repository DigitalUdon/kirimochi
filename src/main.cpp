#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtLogging>
#include <QtWebEngineQuick>

int main(int argc, char* argv[]) {
    qSetMessagePattern("[%{time hh:mm:ss.zzz}] %{type} %{function} - %{message}");

    qputenv("QT_QUICK_CONTROLS_UNIVERSAL_THEME", "Dark");
    QQuickStyle::setStyle("Universal");

    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Kirimochi", "Main");

    return QCoreApplication::exec();
}
