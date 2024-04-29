#ifndef SETTINGSPROVIDER_H
#define SETTINGSPROVIDER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <mime_dbus_framework.h>

class SettingsProvider : public QObject
{
    Q_OBJECT
private:
    QQmlApplicationEngine& engine;
    MyFramework::DesktopEntryEditor& frame;
    MyFramework::ApplicationInterface& interface;
    const QUrl& main_url;
    const QString& actual_service_name;
public:
    explicit SettingsProvider(QQmlApplicationEngine& engine,
                              const QString& service_name,
                              MyFramework::DesktopEntryEditor& framework,
                              MyFramework::ApplicationInterface& interface,
                              const QUrl& main_url,
                              QObject *parent = nullptr);

signals:

public slots:
    void provide_file_settings(QObject* obj, const QUrl& url);
    void accept_settings(const QString& service, const QString& executable, const QString& mime_types);
    void accept_drop(const QVariant& drop);

};

#endif // SETTINGSPROVIDER_H
