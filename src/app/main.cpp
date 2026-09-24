#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include "LogicController.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    LogicController controller;
    engine.rootContext()->setContextProperty("logicController", &controller);

    // Вказуємо оновлений шлях до інтерфейсу LogicUI
    const QUrl url(QStringLiteral("qrc:/qt/qml/LogicUI/src/ui/main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "Помилка: не вдалося завантажити інтерфейс з:" << url;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}