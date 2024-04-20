#ifndef MIME_DBUS_FRAMEWRORK_H
#define MIME_DBUS_FRAMEWRORK_H

#include "mime_dbus_framewrork_global.h"

#include <QObject>

class MIME_DBUS_FRAMEWRORK_EXPORT Mime_dbus_framewrork : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString service_name READ getServiceName WRITE setServiceName NOTIFY onServiceNameChanged)


public:
    Mime_dbus_framewrork();
    explicit Mime_dbus_framewrork(QObject* parent);

    const QString& getServiceName() const
    {
        return m_service_name;
    }

    void setServiceName(const QString& new_name)
    {
        m_service_name = new_name;
    }

    Q_INVOKABLE void add_type(const QString type, bool write_to_home_user_local = false);
    Q_INVOKABLE void add_entry();

signals:
    void onServiceNameChanged();
private:
    QString m_service_name;
    QString m_path_to;
};

#endif // MIME_DBUS_FRAMEWRORK_H
