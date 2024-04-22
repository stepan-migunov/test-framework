#include "mime_dbus_framework.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <QCoreApplication>
#include <iostream>

#include <QtDBus/QDBusConnection>

MimeDBusFramework::MimeDBusFramework()
{
}

MimeDBusFramework::MimeDBusFramework(QObject *parent):
    QObject(parent),
    m_service_name(""),
    m_path_to(".")
{

}

void MimeDBusFramework::add_type(const QString type, bool write_to_home_user_local)
{
    QString _path_to = ".";
    if (write_to_home_user_local)
    {
        _path_to = qEnvironmentVariable("HOME") + "/.local/share/applications";
    }
    else
    {
        qWarning() << "Without modifying default desktop entry, changes would not take any effect.";
    }
    if (m_service_name.size() < 1)
    {
        qCritical() << "Could not add type " + type + ": Lack of DBus Service Name.";
        return;
    }
    if (_path_to.lastIndexOf("/") == (_path_to.size() - 1))
    {
        _path_to.chop(1);
    }

    QFile desktop_entry_file(_path_to + "/" + m_service_name + ".desktop");

    if (desktop_entry_file.exists())
    {
        if (not desktop_entry_file.open(QIODevice::ReadWrite))
        {
            qCritical() << "Could not modify file + " + m_service_name + ".desktop";
            std::exit(3);
        }

        QTextStream read_de(&desktop_entry_file);

        QStringList contents;

        qsizetype de_section_line_begin = -1;
        qsizetype de_section_line_end = -1;
        qsizetype de_mimetype_section = -1;

        qsizetype line_number = -1;

        while(!read_de.atEnd())
        {
            line_number++;
            auto line = read_de.readLine(68000);
            contents.append(line);

            if(line.startsWith("[Desktop Entry]")){
                de_section_line_begin = line_number;
                continue;
            }

            if(line.startsWith('[') && de_section_line_begin != -1 && de_section_line_end == -1)
            {
                de_section_line_end = line_number - 1;
                continue;
            }

            if (line.startsWith("MimeType"))
            {
                de_mimetype_section = line_number;
                continue;
            }
        }

        if (de_section_line_begin == -1)
        {
            qWarning() << "This doesn't look like valid Desktop Entry file, \n"
                          "thus I could not add type " + type + " to be supported.\n";
            desktop_entry_file.close();
            return;
        }

        if (de_mimetype_section != -1)
        {
            if(!contents[de_mimetype_section].contains(type))
                contents[de_mimetype_section].append(type + ";");
        }
        else
        {
            if (de_section_line_end == -1)
            {
                contents.append("MimeType=" + type + ";");
            }
            else
            {
                contents.insert(de_section_line_end, "MimeType=" + type + ";");
            }
        }
        if(desktop_entry_file.resize(0))
            desktop_entry_file.write(contents.join("\n").toUtf8());
        else
            qCritical() << "Could not write to the file " + m_service_name + ".desktop.";
    }
    else
    {
        qWarning() << "Warning: Could not detect file " + _path_to + "/" + m_service_name + ".desktop.";
        qWarning() << "I will try to write this file myself, and you should fill in necessary parts.";
        if(!desktop_entry_file.open(QIODevice::OpenModeFlag::NewOnly
                                | QIODevice::OpenModeFlag::ReadWrite ))
        {
            qCritical() << "Could not create Desktop Entry file";
            QCoreApplication::exit(5);
        }
        desktop_entry_file.setPermissions(QFile::Permission::ExeGroup
                                          | QFile::Permission::ExeOther
                                          | QFile::Permission::ExeOwner
                                          | QFile::Permission::ExeUser
                                          | QFile::Permission::ReadOwner
                                          | QFile::Permission::WriteOwner);
        QStringList contents;
        contents.append(QString("[Desktop Entry]"));
        contents.append(QString("Name=") + m_service_name);
        contents.append(QString("Type=Application"));
        contents.append(QString("MimeType=") + type + QString(";"));
        contents.append(QString("DBusActivatable=true"));
        contents.append(QString("Exec=") + QCoreApplication::arguments().at(0) + " %F");

        desktop_entry_file.write(contents.join('\n').toUtf8());
    }
}

bool ApplicationInterface::register_service(const QString &serviceName)
{
    if (serviceName.endsWith(".service"))
    {
        m_service_name = serviceName.chopped(QString(".service").length());
    }
    else
    {
        m_service_name = serviceName;
    }
    QFile dbus_service_file(qEnvironmentVariable("HOME") + "/.local/share/dbus-1/services/" + m_service_name + ".service");
    if (!dbus_service_file.exists())
    {
        qWarning() << "Could not find .service, associated with " + serviceName;
        qWarning() << "I will try to write this file myself, and you should fill in necessary parts.";
        if(!dbus_service_file.open(QIODevice::OpenModeFlag::NewOnly
                                | QIODevice::OpenModeFlag::ReadWrite ))
        {
            qCritical() << "Could not create DBus Service file";
            QCoreApplication::exit(4);
        }
        dbus_service_file.setPermissions(QFile::Permission::ExeGroup
                                          | QFile::Permission::ExeOwner
                                          | QFile::Permission::ExeUser
                                          | QFile::Permission::ReadOwner
                                          | QFile::Permission::WriteOwner);
        QStringList contents;
        contents.append("[D-BUS Service]");
        contents.append("Name=" + m_service_name);
        contents.append("Exec=" + QCoreApplication::arguments().at(0) + " %F");

        dbus_service_file.write(contents.join('\n').toUtf8());
    }
    QDBusConnection::sessionBus().registerService(m_service_name);
    return QDBusConnection::sessionBus().registerService(m_service_name);
}

bool ApplicationInterface::registerObject(const QString &objectPath, QObject *object)
{
    if ((objectPath.length() == 0) && (m_service_name.length() == 0))
    {
        qCritical() << "No object path provided";
        QCoreApplication::exit(2);
    }

    QString _objectPath = "/";
    if (objectPath.length() > 1 && objectPath.contains('.'))
    {
        _objectPath.append(objectPath.split(".").join("/"));
    }
    else if (objectPath.length() > 0 && objectPath[0] == '/')
    {
        _objectPath = objectPath;
    }
    else
    {
        _objectPath.append(m_service_name.split(".").join("/"));
    }
    QObject* _object = (object != nullptr ? object : m_object);
    return QDBusConnection::sessionBus().registerObject(_objectPath, _object);
}

ApplicationInterface::~ApplicationInterface()
{

}

ApplicationInterface::ApplicationInterface(QObject *object):
    QDBusAbstractAdaptor(object == nullptr ? (object = new QObject) : object)
{
    m_object = object;
    m_activate = activate_function_type{};
    m_open = open_function_type{};
    m_activate_action = activate_action_function_type{};
}


void ApplicationInterface::Activate(const QMap<QString, QVariant> &platform_data)
{
    m_activate(platform_data);
}

void ApplicationInterface::Open(const QList<QString> &uris, const QMap<QString, QVariant> &platform_data)
{
    m_open(uris, platform_data);
}

void ApplicationInterface::ActivateAction(const QString &action_name, const QList<QVariant> parameter, const QMap<QString, QVariant> &platform_data)
{
    m_activate_action(action_name, parameter, platform_data);
}


