#include "mime_dbus_framewrork.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QString>

Mime_dbus_framewrork::Mime_dbus_framewrork()
{
}

Mime_dbus_framewrork::Mime_dbus_framewrork(QObject *parent):
    QObject(parent),
    m_service_name(""),
    m_path_to(".")
{

}

void Mime_dbus_framewrork::add_type(const QString type, bool write_to_home_user_local)
{
    QString _path_to = ".";
    if (write_to_home_user_local)
    {
        _path_to = qEnvironmentVariable("HOME") + "/.local/share/applications";
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
            std::exit(1);
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
        qCritical() << "Warning: Could not detect file " + _path_to + "/" + m_service_name + ".desktop.";

    }
}

void Mime_dbus_framewrork::add_entry()
{

}
