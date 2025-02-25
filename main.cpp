#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "gps.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    GPS gps;
    gps.start(); // Open serial port and enable GPS

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gps", &gps);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
