#ifndef APPLICATIONINTERFACE_H
#define APPLICATIONINTERFACE_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtDBus>
#include <iostream>

class applicationInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Application")

public:
    applicationInterface(QObject* object = nullptr);
    virtual ~applicationInterface();
public slots:
    Q_NOREPLY void Activate(const QMap<QString, QVariant>& platform_data);
    Q_NOREPLY void Open(const QList<QString>& uris,
                                const QMap<QString, QVariant>& platform_data);
    Q_NOREPLY void ActivateAction(const QString& action_name,
                                          const QList<QVariant> parameter,
                                          const QMap<QString, QVariant>& platform_data);
public:

    QGuiApplication* app;
    QQmlApplicationEngine* engine;
};

#endif // APPLICATIONINTERFACE_H
