#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "CANBackend.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Create backend and expose to QML
    CANBackend backend;
    engine.rootContext()->setContextProperty("canBackend", &backend);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("can_frame_visualizer", "Main");

    return app.exec();
}