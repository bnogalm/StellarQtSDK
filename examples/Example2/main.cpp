#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "stellargateway.h"
#include "wallet.h"
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    //we register the types so they can be used from QML importing "Example 2.0". From Qt5.15 there will be a new and better way to register types.
    qmlRegisterType<StellarGateway>("Example",2,0, "StellarGateway");
    qmlRegisterType<Wallet>("Example",2,0, "Wallet");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    //connection that exits application with -1 if there is a problem loading main.qml
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    //here the gui is loaded.
    engine.load(url);

    //here is when all the signals and slots starts to execute.
    return app.exec();
}
