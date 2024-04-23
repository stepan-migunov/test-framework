#ifndef MIME_DBUS_FRAMEWORK_H
#define MIME_DBUS_FRAMEWORK_H

#include "mime_dbus_framework_global.h"

#include <QObject>

#include <QtDBus/QDBusAbstractAdaptor>

namespace MyFramework
{

class ApplicationInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Application")

    QString m_service_name;
    QString m_object_name;
    QObject* m_object;

    typedef std::function<void(const QMap<QString, QVariant>&)> activate_function_type;
    typedef std::function<void(const QList<QString>& uris,
                             const QMap<QString, QVariant>& platform_data)> open_function_type;
    typedef     std::function<void(const QString& action_name,
                                  const QList<QVariant> parameter,
                                  const QMap<QString, QVariant>& platform_data)> activate_action_function_type;

    activate_function_type          m_activate;
    open_function_type              m_open;
    activate_action_function_type   m_activate_action;

public slots:
    Q_NOREPLY void Activate(const QMap<QString, QVariant>& platform_data);
    Q_NOREPLY void Open(const QList<QString>& uris,
                                const QMap<QString, QVariant>& platform_data);
    Q_NOREPLY void ActivateAction(const QString& action_name,
                                          const QList<QVariant> parameter,
                                          const QMap<QString, QVariant>& platform_data);
public:
    void setActivate(activate_function_type activate) { m_activate = activate; }
    void setOpen(open_function_type open) { m_open = open; }
    void setActivateAction(activate_action_function_type activate_action) { m_activate_action = activate_action; }

    ApplicationInterface(QObject* object = nullptr);
    bool register_service(const QString& serviceName, const QString& serviceExecutable);
    bool registerObject(const QString& objectPath = "/", QObject* object = nullptr);



    virtual ~ApplicationInterface();
};

class MIME_DBUS_FRAMEWORK_EXPORT MimeDBusFramework : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString service_name READ getServiceName WRITE setServiceName NOTIFY onServiceNameChanged)


public:
    MimeDBusFramework();
    explicit MimeDBusFramework(QObject* parent);

    const QString& getServiceName() const
    {
        return m_service_name;
    }

    void setServiceName(const QString& new_name)
    {
        m_service_name = new_name;
    }

    Q_INVOKABLE void add_type(const QString& type, const QString& executablePath);


signals:
    void onServiceNameChanged();
private:
    QString m_service_name;
};

}

#endif // MIME_DBUS_FRAMEWORK_H
