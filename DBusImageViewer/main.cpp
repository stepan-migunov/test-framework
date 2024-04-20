#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QLocale>
#include <QFile>
#include <QTranslator>
#include <iostream>
#include <mime_dbus_framewrork.h>

#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus>

#include "applicationInterface.h"



int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QObject* obj = new QObject;

    Mime_dbus_framewrork m(&app);
    QQmlApplicationEngine engine;
    applicationInterface a(obj);
    a.app = &app;
    a.engine = &engine;


    QDBusConnection::sessionBus().registerObject("/org/stepanmigunov/DBusImageViewer", obj);
    if (!QDBusConnection::sessionBus().registerService(QString("org.stepanmigunov.DBusImageViewer")))
    {
        qCritical() << "could not register service";
        std::exit(-1);
    }

    m.setServiceName("org.stepanmigunov.DBusImageViewer");

    m.add_type("text/plain", true);
    m.add_entry();

    return app.exec();
}


