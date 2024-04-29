#include "mime_dbus_framework.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <QCoreApplication>
#include <iostream>

#include <QtDBus/QDBusConnection>

QStringList file_contents(QTextStream& stream)
{
    QStringList result;
    while(not stream.atEnd())
    {
        result.append(stream.readLine());
    }
    return result;
}

QString remove_substring_from(const QString& substring, const QString& from)
{
    QString result = "";
    result += from.sliced(0,from.indexOf(substring));
    result += from.sliced(from.indexOf(substring) + substring.length());

    return result;

}

using namespace MyFramework;

DesktopEntryEditor::DesktopEntryEditor()
{
}

DesktopEntryEditor::DesktopEntryEditor(QObject *parent):
    QObject(parent)
{

}

bool DesktopEntryEditor::set_name(const QString &service_name, const QString &language, const QString &name)
{
    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");
    auto names = get_names(service_name);

    if (names.size() < 1)
    {
        qCritical() << "Error setting service name: No names provided for service " + service_name + ". Thus this action deduced to be incorrect";
        return false;
    }

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error setting service name: File " + path_to_file + " does not exists";
        return false;
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error setting service name: Could not open file " + path_to_file;
        return false;
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section_begin = -1;
    qsizetype desktop_entry_section_end = -1;
    qsizetype last_action_name = -1;
    qsizetype existing_name = -1;

    bool default_lang = (language == QString("[default]"));

    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section_begin = line;
            continue;
        }

        if (contents[line].startsWith("Name") && desktop_entry_section_begin != -1 && desktop_entry_section_end == -1)
        {
            if(default_lang && contents[line].startsWith(QString("Name=")))
            {
                existing_name = line;
                break;
            }

            if (contents[line].contains(language))
                existing_name = line;
            last_action_name = line;
            continue;
        }

        if (contents[line]. startsWith('[') && desktop_entry_section_begin != -1 && desktop_entry_section_end == -1)
        {
            desktop_entry_section_end = line - 1;
            break;
        }
    }

    if (desktop_entry_section_begin == -1)
    {
        qCritical() << "Error setting action name: " + path_to_file + " is not a valid Desktop Entry file";
        file.close();
        return false;
    }

    desktop_entry_section_end = desktop_entry_section_end == -1 ? contents.size() - 1 : desktop_entry_section_end;
    if (existing_name != -1 && existing_name >= desktop_entry_section_begin && existing_name <= desktop_entry_section_end)
    {
        auto old_line = contents[existing_name];
        auto new_line = old_line.sliced(0, old_line.indexOf('=') + QString("=").length()) + name;
        contents[existing_name] = new_line;
    }
    else
    {
        contents.insert(last_action_name, QString("Name") + (default_lang ? QString("") : language) + QString("=") + name);
    }

    if (not file.resize(0))
    {
        qCritical() << "Error setting service name: Could not write to file " + path_to_file;
        file.close();
        return false;
    }
    auto _ = file.write(contents.join('\n').toUtf8());
    if (_ < contents.join('\n').toUtf8().size())
    {
        qCritical() << "Error setting service name: Could not write to file " + path_to_file;
        file.close();
        return false;
    }

    return true;
}

QString DesktopEntryEditor::get_name(const QString &service_name, const QString &language)
{
    auto names = get_names(service_name);

    if (names.contains(language))
    {
        return names[language];
    }
    else
    {
        return "";
    }
}

QMap<QString, QString> DesktopEntryEditor::get_names(const QString &service_name)
{
    auto path_to_file = qEnvironmentVariable("HOME") + QString("/.local/share/applications/") + service_name + QString(".desktop");

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error getting service names: Could not find file " + path_to_file;
        return {};
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error getting service names: Could not read from file " + path_to_file;
        return {};
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section_begin = -1;
    qsizetype desktop_entry_section_end = -1;


    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section_begin = line;
            continue;
        }


        if (contents[line].startsWith('[') && desktop_entry_section_begin != -1 && desktop_entry_section_end == -1)
        {
            desktop_entry_section_end = line - 1;
            break;
        }
    }

    if (desktop_entry_section_begin == -1)
    {
        qWarning() << "Error getting service's names: This is not a valid Desktop Entry file";
        return {};
    }

    auto result = QMap<QString, QString>{};

    for (auto i = desktop_entry_section_begin; desktop_entry_section_end == -1 ? i < contents.size() : i < desktop_entry_section_end; i++)
    {
        if (contents[i].startsWith("Name"))
        {
            const auto& line = contents[i];
            auto lang = line.startsWith("Name[") ? line.sliced(line.indexOf('['),4) : QString("[default]");

            auto name = line.sliced(line.indexOf('=') + QString("=").length());

            result.insert(lang, name);
        }
    }
    return result;
}

bool DesktopEntryEditor::add_type(const QString& service_name, const QString& type)
{
    if (get_types(service_name).contains(type))
    {
        return true;
    }

    QString _path_to = qEnvironmentVariable("HOME") + "/.local/share/applications";

    if (service_name.size() < 1)
    {
        qCritical() << "Error adding type: " + type + ": Lack of DBus Service Name.";
        return false;
    }
    if (_path_to.lastIndexOf("/") == (_path_to.size() - 1))
    {
        _path_to.chop(1);
    }

    QFile desktop_entry_file(_path_to + "/" + service_name + ".desktop");

    if (desktop_entry_file.exists())
    {
        if (not desktop_entry_file.open(QIODevice::ReadWrite))
        {
            qCritical() << "Error adding type: Could not modify file + " + service_name + ".desktop";
            return false;
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

            if (line.startsWith("MimeType") && de_section_line_begin != -1 && de_section_line_end == -1)
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
            return false;
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
        if(desktop_entry_file.resize(0)){
            auto _ = contents.join("\n");
            if (_.size() > desktop_entry_file.write(_.toUtf8()))
            {
                qCritical() << "Error adding type: Could not write to the file " + service_name + ".desktop.";
                return false;
            }
        }
        else{
            qCritical() << "Error adding type: Could not write to the file " + service_name + ".desktop.";
            return false;
        }

    }
    else
    {
        qWarning() << "Warning: Could not detect file " + _path_to + "/" + service_name + ".desktop.";
        qWarning() << "I will try to write this file myself, and you should fill in necessary parts.";
        if(!desktop_entry_file.open(QIODevice::OpenModeFlag::NewOnly
                                | QIODevice::OpenModeFlag::ReadWrite ))
        {
            qCritical() << "Error adding type: Could not create Desktop Entry file";
            return false;
        }
        desktop_entry_file.setPermissions(QFile::Permission::ExeGroup
                                          | QFile::Permission::ExeOther
                                          | QFile::Permission::ExeOwner
                                          | QFile::Permission::ExeUser
                                          | QFile::Permission::ReadOwner
                                          | QFile::Permission::WriteOwner);
        QStringList contents;
        contents.append(QString("[Desktop Entry]"));
        contents.append(QString("Name=") + service_name);
        contents.append(QString("Type=Application"));
        contents.append(QString("MimeType=") + type + QString(";"));
        contents.append(QString("DBusActivatable=true"));

        auto _ = contents.join('\n').toUtf8();
        if (_.size() > desktop_entry_file.write(_))
        {
            qCritical() << "Error adding type: Could not write to the file " + service_name + ".desktop.";
            return false;
        }
    }
    return true;
}


bool DesktopEntryEditor::set_service_executable(const QString &service_name, const QString &executablePath)
{
    QString _path_to = qEnvironmentVariable("HOME") + "/.local/share/applications";

    if (service_name.size() < 1)
    {
        qCritical() << "Error setting executable: Lack of DBus Service Name.";
        return false;
    }

    if (_path_to.lastIndexOf("/") == (_path_to.size() - 1))
    {
        _path_to.chop(1);
    }

    QFile desktop_entry_file(_path_to + "/" + service_name + ".desktop");

    if (desktop_entry_file.exists())
    {
        if (not desktop_entry_file.open(QIODevice::ReadWrite))
        {
            qCritical() << "Error setting executable: Could not modify file + " + service_name + ".desktop";
            return false;
        }

        QTextStream read_de(&desktop_entry_file);

        QStringList contents;

        qsizetype de_section_line_begin = -1;
        qsizetype de_section_line_end = -1;
        qsizetype de_exec_section = -1;

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

            if (line.startsWith("Exec") && de_section_line_begin != -1 && de_section_line_end == -1)
            {
                de_exec_section = line_number;
                continue;
            }
        }

        if (de_section_line_begin == -1)
        {
            qWarning() << "Error setting executable: This doesn't look like valid Desktop Entry file, \n";
            desktop_entry_file.close();
            return false;
        }

        if (de_exec_section != -1)
        {
            contents[de_exec_section] = QString("Exec=") + executablePath;
        }
        else
        {
            if (de_section_line_end == -1)
            {
                contents.append("Exec=" + executablePath);
            }
            else
            {
                contents.insert(de_section_line_end, "Exec=" + executablePath);
            }
        }
        if(desktop_entry_file.resize(0))
            desktop_entry_file.write(contents.join("\n").toUtf8());
        else
            qCritical() << "Error setting executable: Could not write to the file " + service_name + ".desktop.";
        return false;
    }
    else
    {
        qWarning() << "Warning: Could not detect file " + _path_to + "/" + service_name + ".desktop.";
        qWarning() << "I will try to write this file myself, and you should fill in necessary parts.";
        if(!desktop_entry_file.open(QIODevice::OpenModeFlag::NewOnly
                                | QIODevice::OpenModeFlag::ReadWrite ))
        {
            qCritical() << "Error setting executable: Could not create Desktop Entry file";
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
        contents.append(QString("Name=") + service_name);
        contents.append(QString("Type=Application"));
        contents.append(QString("Exec=") + executablePath);
        contents.append(QString("DBusActivatable=true"));

        auto result = desktop_entry_file.write(contents.join('\n').toUtf8());
        if (result < 0 || result < contents.join('\n').toUtf8().length())
            return false;
        else
            return true;
    }
}

QString DesktopEntryEditor::get_service_executable(const QString &service_name)
{
    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error getting executable: File " + path_to_file + " does not exists";
            return "";
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error getting executable: Could not open file " + path_to_file;
        return "";
    }

    QTextStream input(&file);

    auto contents = file_contents(input);


    qsizetype desktop_entry_section_begin = -1;
    qsizetype desktop_entry_end = -1;
    qsizetype exec_line = -1;
    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section_begin = line;
            continue;
        }

        if (contents[line].startsWith('[') && desktop_entry_end == -1)
        {
            desktop_entry_end = line - 1;
            continue;
        }

        if (contents[line].startsWith(QString("Exec=")) && desktop_entry_section_begin != -1 && desktop_entry_end == -1)
        {
            exec_line = line;
            break;
        }
    }

    if (exec_line == -1)
    {
        return "";
    }
    else
    {
        return contents[exec_line].sliced(QString("Exec=").length());
    }
}

bool DesktopEntryEditor::set_action_name(const QString &service_name, const QString &action, const QString &language, const QString &action_name)
{
    if (not get_actions(service_name).contains(action))
    {
        qWarning() << "Error setting action name: This service does not have action " + action;
        return false;
    }

    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");
    auto names = get_action_names(service_name, action);

    if (names.size() < 1)
    {
        qCritical() << "Error setting action name: No names provided for action " + action + ". Thus this action deduced to be incorrect";
        return false;
    }

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error setting action name: File " + path_to_file + " does not exists";
        return false;
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error setting action name: Could not open file " + path_to_file;
        return false;
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section = -1;
    qsizetype action_section_begin = -1;
    qsizetype action_section_end = -1;
    qsizetype last_action_name = -1;
    qsizetype existing_name = -1;

    bool default_lang = language == QString("[default]");

    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section = line;
            continue;
        }

        if (contents[line] == QString("[Desktop Action ") + action + QString("]"))
        {
            action_section_begin = line;
            continue;
        }

        if (contents[line].startsWith("Name") && action_section_begin != -1 && action_section_end == -1)
        {
            if(default_lang && contents[line].startsWith(QString("Name=")))
            {
                existing_name = line;
                break;
            }

            if (contents[line].contains(language))
                existing_name = line;
            last_action_name = line;
            continue;
        }

        if (contents[line]. startsWith('[') && action_section_begin != -1 && action_section_end == -1)
        {
            action_section_end = line - 1;
            break;
        }
    }

    if (desktop_entry_section == -1)
    {
        qCritical() << "Error setting action name: " + path_to_file + " is not a valid Desktop Entry file";
        file.close();
        return false;
    }

    action_section_end = action_section_end == -1 ? contents.size() - 1 : action_section_end;
    if (existing_name != -1 && existing_name >= action_section_begin && existing_name <= action_section_end)
    {
        auto old_line = contents[existing_name];
        auto new_line = old_line.sliced(0, old_line.indexOf('=') + QString("=").length()) +
                action_name;
        contents[existing_name] = new_line;
    }
    else
    {
        contents.insert(last_action_name, QString("Name") + (default_lang ? QString("") : language) + QString("=") + action_name);
    }

    if (not file.resize(0))
    {
        qCritical() << "Error setting action name: Could not write to file " + path_to_file;
        file.close();
        return false;
    }
    auto _ = file.write(contents.join('\n').toUtf8());
    if (_ < contents.join('\n').toUtf8().size())
    {
        qCritical() << "Error setting action name: Could not write to file " + path_to_file;
        file.close();
        return false;
    }

    return true;
}

QString DesktopEntryEditor::get_action_name(const QString &service_name, const QString &action, const QString &language)
{
    auto names = get_action_names(service_name, action);

    if (names.contains(language))
    {
        return names[language];
    }
    else
    {
        return "";
    }

}


QMap<QString, QString> DesktopEntryEditor::get_action_names(const QString &service_name, const QString &action)
{
    if (not get_actions(service_name).contains(action))
    {
        qWarning() << QString("Error getting action's names: Service ") + service_name + QString(" does not contain action ") + action;
        return {};
    }

    auto path_to_file = qEnvironmentVariable("HOME") + QString("/.local/share/applications/") + service_name + QString(".desktop");

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error getting action names: Could not find file " + path_to_file;
        return {};
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error getting action names: Could not read from file " + path_to_file;
        return {};
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section = -1;
    qsizetype action_section_begin = -1;
    qsizetype action_section_end = -1;

    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section = line;
            continue;
        }

        if (contents[line] == QString("[Desktop Action ") + action + QString("]") )
        {
            action_section_begin = line;
            continue;
        }

        if (contents[line].startsWith('[') && action_section_begin != -1 && action_section_end == -1)
        {
            action_section_end = line - 1;
            break;
        }
    }

    if (desktop_entry_section == -1)
    {
        qWarning() << "Error getting action's names: This is not a valid Desktop Entry file";
        return {};
    }

    auto result = QMap<QString, QString>{};

    for (auto i = action_section_begin; action_section_end == -1 ? i < contents.size() : i < action_section_end; i++)
    {
        if (contents[i].startsWith("Name"))
        {
            const auto& line = contents[i];
            auto lang = line.startsWith("Name[") ? line.sliced(line.indexOf('['),4) : QString("[default]");

            auto name = line.sliced(line.indexOf('=') + QString("=").length());

            result.insert(lang, name);
        }
    }
    return result;

}

bool DesktopEntryEditor::remove_action_name(const QString &service_name, const QString &action, const QString &language, const QString& action_name)
{
    if (not get_actions(service_name).contains(action))
    {
        qWarning() << "Error removing action's name: This service does not have action " + action;
        return false;
    }

    if (language == QString("[default]"))
    {
        qWarning() << "Error removing action's name: Could not remove default name";
        return false;
    }

    if (not get_action_names(service_name, action).contains(action_name))
    {
        return true;
    }

    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");
    auto names = get_action_names(service_name, action);

    if (names.size() < 1)
    {
        qCritical() << "Error removing action name: No names provided for action " + action + ". Thus this action deduced to be incorrect";
        return false;
    }

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error removing action name: File " + path_to_file + " does not exists";
        return false;
    }

    if(not file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error removing action name: Could not open file " + path_to_file;
        return false;
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section = -1;
    qsizetype action_section_begin = -1;
    qsizetype action_section_end = -1;
    qsizetype existing_name = -1;



    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section = line;
            continue;
        }

        if (contents[line] == QString("[Desktop Action ") + action + QString("]"))
        {
            action_section_begin = line;
            continue;
        }

        if (contents[line].startsWith("Name") && action_section_begin != -1 && action_section_end == -1)
        {
            if (contents[line].contains(language))
            {
                existing_name = line;
                break;
            }
        }

        if (contents[line]. startsWith('[') && action_section_begin != -1 && action_section_end == -1)
        {
            action_section_end = line - 1;
            break;
        }
    }

    if (desktop_entry_section == -1)
    {
        qCritical() << "Error removing action name: " + path_to_file + " is not a valid Desktop Entry file";
        file.close();
        return false;
    }

    action_section_end = action_section_end == -1 ? contents.size() - 1 : action_section_end;

    if (existing_name != -1 && existing_name >= action_section_begin && existing_name <= action_section_end)
    {
        contents.remove(existing_name);
    }
    else
    {
        qWarning() << "Error removing action name: Could not find name "
                      + action_name + " under action " + action + " in service " + service_name;
    }

    if (not file.resize(0))
    {
        qCritical() << "Error removing action name: Could not write to file " + path_to_file;
        file.close();
        return false;
    }
    auto _ = file.write(contents.join('\n').toUtf8());
    if (_ < contents.join('\n').toUtf8().size())
    {
        qCritical() << "Error removing action name: Could not write to file " + path_to_file;
        file.close();
        return false;
    }

    return true;
}

QString DesktopEntryEditor::get_action_exec(const QString &service_name, const QString &action)
{
    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error getting action executable: File " + path_to_file + " does not exists";
            return "";
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error getting action executable: Could not open file " + path_to_file;
        return "";
    }

    QTextStream input(&file);

    auto contents = file_contents(input);


    qsizetype desktop_entry_section_begin = -1;
    qsizetype action_section_begin = -1;
    qsizetype action_section_end = -1;
    qsizetype exec_line = -1;


    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Action ") + action + QString("]"))
        {
            desktop_entry_section_begin = line;
            continue;
        }

        if (contents[line].startsWith('[') && action_section_begin != -1 && action_section_end == -1)
        {
            action_section_end = line;
        }

        if (contents[line].startsWith(QString("Exec=")) && action_section_begin != -1 && action_section_end == -1)
        {
            exec_line = line;
            break;
        }
    }

    if (desktop_entry_section_begin == -1)
    {
        qCritical() << "Error getting action executable: " + path_to_file + " is not a valid Desktop Entry File";
        return "";
    }

    if (exec_line == -1)
    {
        qWarning() << "Error getting action executable: Could not find \"Exec\" key in action " + action;
        return "";
    }
    else
    {
        return contents[exec_line].sliced(QString("Exec=").length());
    }
}

bool DesktopEntryEditor::set_action_exec(const QString& service_name, const QString &action, const QString &action_exec)
{
    if(not get_actions(service_name).contains(action))
    {
        qWarning() << "Error setting action's executable: " + service_name + " service does not have action " + action;
        return false;
    }

    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error setting action executable: File " + path_to_file + " does not exists";
        return false;
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error setting action executable: Could not open file " + path_to_file;
        return false;
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section = -1;
    qsizetype action_section_begin = -1;
    qsizetype action_section_end = -1;
    qsizetype exec_line = -1;

    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section = line;
            continue;
        }

        if (contents[line] == QString("[Desktop Action ") + action + QString("]"))
        {
            action_section_begin = line;
            continue;
        }

        if (contents[line].startsWith("Exec=") && action_section_begin != -1 && action_section_end == -1)
        {
            exec_line= line;
            break;
        }

        if (contents[line]. startsWith('[') && action_section_begin != -1 && action_section_end == -1)
        {
            action_section_end = line - 1;
            break;
        }
    }

    if (desktop_entry_section == -1)
    {
        qCritical() << "Error setting action executable: " + path_to_file + " is not a valid Desktop Entry file";
        file.close();
        return false;
    }

    if (not (action_section_end != -1 && exec_line < action_section_end && exec_line > action_section_begin))
    {
        qCritical() << "Error setting action executable: " + action + " does not have \"Exec\" key";
        return false;
    }

    if (exec_line != -1)
    {
        contents[exec_line] = "Exec=" + action_exec;
    }
    else
    {
        qWarning() << "Error setting action's executable: " + service_name + " service does not have \"Exec\" key";
        return false;
    }

    if (not file.resize(0))
    {
        qCritical() << "Error setting action executable: Could not write to file " + path_to_file;
        file.close();
        return false;
    }
    auto _ = file.write(contents.join('\n').toUtf8());
    if (_ < contents.join('\n').toUtf8().size())
    {
        qCritical() << "Error setting action executable: Could not write to file " + path_to_file;
        file.close();
        return false;
    }

    return true;
}

QStringList DesktopEntryEditor::get_actions(const QString &service_name)
{
    QString path_to_file = qEnvironmentVariable("HOME") +
                            QString("/.local/share/applications/") +
                            service_name +
                            QString(".desktop");

    QFile desktop_entry_file(path_to_file);

    if (not desktop_entry_file.exists())
    {
        return {};
    }

    if (not desktop_entry_file.open(QIODevice::ReadOnly | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error getting actions: Could not read file + " + service_name + ".desktop";
        return {};
    }

    QTextStream read_de(&desktop_entry_file);

    QStringList contents = file_contents(read_de);

    qsizetype desktop_entry_section = -1;
    qsizetype actions_section = -1;

    for(qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section = line;
        }
        if (contents[line].startsWith("Actions="))
        {
            actions_section = line;
            break;
        }
    }

    if (desktop_entry_section == -1)
    {
        qCritical() << "Error getting actions: This is not valid Desktop Entry file";
        desktop_entry_file.close();
        return {};
    }

    if (actions_section == -1)
    {
        desktop_entry_file.close();
        return {};
    }

    auto actions = contents[actions_section].sliced(QString("Actions=").length());
    desktop_entry_file.close();
    return actions.length() > 0 ? actions.split(';',Qt::SkipEmptyParts) : QList<QString>{};
}

bool DesktopEntryEditor::add_action(const QString &service_name,
                                    const QString& action,
                                    const QString &action_name,
                                    const QString &action_exec,
                                    const QString &action_icon)
{
    if (get_actions(service_name).contains(action))
    {
        qWarning() << "Error adding new action: " + service_name + " service already have " + action + " action";
        return false;
    }

    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error adding actions: File " + path_to_file + " does not exists";
        return false;
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error adding actions: Could not open file " + path_to_file;
        return false;
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section = -1;
    qsizetype actions_line = -1;

    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section = line;
            continue;
        }

        if (contents[line].startsWith(QString("Actions=")))
        {
            actions_line = line;
            break;
        }
    }

    if (desktop_entry_section == -1)
    {
        qCritical() << "Error adding actions: " + path_to_file + " is not a valid Desktop Entry file";
        file.close();
        return false;
    }

    if (actions_line != -1)
    {
        contents[actions_line].append(action + QString(";"));
    }
    else
    {
        contents.append(QString("Actions=") + action + QString(";"));
    }

    contents.append("");
    contents.append(QString("[Desktop Action ") + action + QString("]"));
    contents.append(QString("Name=") + action_name);
    contents.append(QString("Exec=") + action_exec);
    if (action_icon.size() > 0)
    {
        contents.append(QString("Icon=") + action_icon);
    }

    if (not file.resize(0))
    {
        qCritical() << "Error adding actions: Could not write to file " + path_to_file;
        file.close();
        return false;
    }
    auto _ = file.write(contents.join('\n').toUtf8());
    if (_ < contents.join('\n').toUtf8().size())
    {
        qCritical() << "Error adding actions: Could not write to file " + path_to_file;
        file.close();
        return false;
    }

    return true;
}

bool DesktopEntryEditor::remove_action(const QString& service_name, const QString &action)
{
    if (not get_actions(service_name).contains(action))
    {
        qWarning() << "Error removing action: " + service_name + " does not contain action " + action;
        return true;
    }

    auto path_to_file = qEnvironmentVariable("HOME") + "/.local/share/applications/" + service_name + QString(".desktop");

    auto file = QFile(path_to_file);

    if(not file.exists())
    {
        qCritical() << "Error removing actions: File " + path_to_file + " does not exists";
        return false;
    }

    if(not file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error removing actions: Could not open file " + path_to_file;
        return false;
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    qsizetype desktop_entry_section = -1;
    qsizetype actions_key = -1;
    qsizetype action_section_begin = -1;
    qsizetype action_section_end = -1;

    for (qsizetype line = 0; line < contents.size(); line++)
    {
        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section = line;
            continue;
        }

        if (contents[line].contains(QString("Actions=")))
        {
            actions_key = line;
            continue;
        }

        if (contents[line] == QString("[Desktop Action ") + action + QString("]"))
        {
            action_section_begin = line;
            continue;
        }

        if (contents[line]. startsWith('[') && action_section_begin != -1 && action_section_end == -1)
        {
            action_section_end = line;
            break;
        }

    }

    if (desktop_entry_section == -1)
    {
        qCritical() << "Error removing actions: " + path_to_file + " is not a valid Desktop Entry file";
        file.close();
        return false;
    }

    contents[actions_key] = remove_substring_from(action + QString(";"), contents[actions_key]);

    auto section_begin = contents.begin() + action_section_begin;
    auto section_end = (action_section_end == -1) ? contents.end() : contents.begin() + action_section_end;

    if(action_section_begin != -1)
        contents.erase(section_begin, section_end);
    else
        qWarning() << "Could not find action's entry";

    if (not file.resize(0))
    {
        qCritical() << "Error removing actions: Could not write to file " + path_to_file;
        file.close();
        return false;
    }
    auto _ = file.write(contents.join('\n').toUtf8());
    if (_ < contents.join('\n').toUtf8().size())
    {
        qCritical() << "Error removing actions: Could not write to file " + path_to_file;
        file.close();
        return false;
    }

    return true;
}


QList<QString> DesktopEntryEditor::get_types(const QString &service_name)
{
    QString _path_to = qEnvironmentVariable("HOME") + "/.local/share/applications";

    if (service_name.size() < 1)
    {
        return {};
    }

    if (_path_to.lastIndexOf("/") == (_path_to.size() - 1))
    {
        _path_to.chop(1);
    }

    QFile desktop_entry_file(_path_to + "/" + service_name + ".desktop");

    if (desktop_entry_file.exists())
    {
        if (not desktop_entry_file.open(QIODevice::ReadOnly | QIODevice::OpenModeFlag::ExistingOnly))
        {
            qCritical() << "Error getting types: Could not read file + " + service_name + ".desktop";
            return {};
        }

        QTextStream read_de(&desktop_entry_file);

        QStringList contents;

        qsizetype de_mime_section = -1;

        qsizetype line_number = -1;

        while(!read_de.atEnd())
        {
            line_number++;
            auto line = read_de.readLine(68000);
            contents.append(line);


            if (line.startsWith("MimeType"))
            {
                de_mime_section = line_number;
                break;
            }
        }

        if (de_mime_section != -1)
        {
            auto types = contents[de_mime_section].sliced(QString("MimeType=").length());
            desktop_entry_file.close();
            return types.length() > 0 ? types.split(";", Qt::SkipEmptyParts) : QList<QString>{};
        }
        else
        {
            desktop_entry_file.close();
            return {};
        }
    }
    else
    {
        return {};
    }
}


bool DesktopEntryEditor::remove_type(const QString &service_name, const QString &type)
{
    auto types = get_types(service_name);
    if(not types.contains(type))
    {
        return true;
    }

    auto file_path = qEnvironmentVariable("HOME") + "/.local/share/applications/";
    auto _service_name = service_name;
    if (service_name.endsWith(".desktop"))
    {
        file_path += service_name;
        _service_name = service_name.chopped(QString(".desktop").length());
    }
    else
    {
        file_path += service_name + QString(".desktop");
    }

    auto file = QFile(file_path);

    if (not file.exists())
    {
        qCritical() << "Error removing type: File " + file_path + " does not exist";
        return false;
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadWrite | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error removing type: File " + file_path + " could not be modified";
        return false;
    }

    QTextStream input(&file);
    QStringList contents = file_contents(input);
    qsizetype desktop_entry_section_begin = -1;
    qsizetype mimetype_line = -1;

    for (qsizetype line = 0; line < contents.size(); line++)
    {

        if (contents[line] == QString("[Desktop Entry]"))
        {
            desktop_entry_section_begin = line;
            continue;
        }

        if (contents[line].startsWith(QString("MimeType=")))
        {
            mimetype_line = line;
            break;
        }
    }

    if (desktop_entry_section_begin == -1)
    {
        qCritical() << "Error removing type: File " + file_path + " is not valid Desktop Entry file";
        file.close();
        return false;
    }

    if (mimetype_line == -1)
    {
        file.close();
        return true;
    }

    if (not contents[mimetype_line].contains(type))
    {
        file.close();
        return true;
    }

    auto old_line = contents[mimetype_line];
    auto new_line = remove_substring_from(type + QString(";"), old_line);
    contents[mimetype_line] = new_line;

    if(not file.resize(0)){
        qCritical() << "Error removing type: Could not write to file " + file_path;
        file.close();
        return false;
    }

    auto _ = contents.join('\n').toUtf8();
    auto bytes_written = file.write(_);
    if( bytes_written < _.size())
    {
        qCritical() << "Error removing type: Could not write to file " + file_path;
        file.close();
        return false;
    }

    file.close();
    return true;
}


bool ApplicationInterface::registerObject(const QString &objectPath, QObject *object)
{
    if ((objectPath.length() == 0))
    {
        qCritical() << "Error registering object: No object path provided";
        return false;
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

bool ApplicationInterface::register_service(const QString &serviceName)
{
    QString _service_name;
    if (serviceName.endsWith(".service"))
    {
        _service_name = serviceName.chopped(QString(".service").length());
    }
    else
    {
        _service_name = serviceName;
    }

    return QDBusConnection::sessionBus().registerService(_service_name);
}

bool ApplicationInterface::set_service_executable(const QString &serviceName, const QString &serviceExecutable)
{
    QString _service_name;
    if (serviceName.endsWith(".service"))
    {
        _service_name = serviceName.chopped(QString(".service").length());
    }
    else
    {
        _service_name = serviceName;
    }

    QFile dbus_service_file(qEnvironmentVariable("HOME") + "/.local/share/dbus-1/services/" + _service_name + ".service");
    if (!dbus_service_file.exists())
    {
        qWarning() << "Could not find .service, associated with " + serviceName;
        qWarning() << "I will try to write this file myself, and you should fill in necessary parts.";

        if(!dbus_service_file.open(QIODevice::OpenModeFlag::NewOnly
                                   | QIODevice::OpenModeFlag::ReadWrite ))
        {
            qCritical() << "Error setting service executable: Could not open DBus Service file";
            return false;
        }
        dbus_service_file.setPermissions(QFile::Permission::ExeGroup
                                         | QFile::Permission::ExeOwner
                                         | QFile::Permission::ExeUser
                                         | QFile::Permission::ReadOwner
                                         | QFile::Permission::WriteOwner);
        QStringList contents;

        contents.append("[D-BUS Service]");
        contents.append("Name=" + _service_name);
        contents.append("Exec=" + serviceExecutable);

        dbus_service_file.write(contents.join('\n').toUtf8());
    }
    else
    {
        if(!dbus_service_file.open(QIODevice::OpenModeFlag::ReadWrite ))
        {
            qCritical() << "Error setting service's executable: Could not access DBus Service file";
            return false;
        }
        QTextStream read_service(&dbus_service_file);

        QStringList contents;

        qsizetype service_section_line_begin = -1;
        qsizetype service_section_line_end = -1;
        qsizetype service_exec_section = -1;

        qsizetype line_number = -1;

        while(!read_service.atEnd())
        {
            line_number++;
            auto line = read_service.readLine(68000);
            contents.append(line);

            if(line.startsWith("[D-BUS Service]")){
                service_section_line_begin = line_number;
                continue;
            }

            if(line.startsWith('[') && service_section_line_begin != -1 && service_section_line_end == -1)
            {
                service_section_line_end = line_number - 1;
                continue;
            }

            if (line.startsWith("Exec") && service_section_line_begin != -1 && service_section_line_end == -1)
            {
                service_exec_section = line_number;
                continue;
            }
        }

        if (service_section_line_begin == -1)
        {
            qWarning() << "Error setting service executable: This is not valid .service file, \n";
            dbus_service_file.close();
            return false;
        }

        if (service_exec_section != -1)
        {
            contents[service_exec_section] = QString("Exec=") + serviceExecutable;
        }
        else
        {
            if (service_section_line_end == -1)
            {
                contents.append("Exec=" + serviceExecutable);
            }
            else
            {
                contents.insert(service_section_line_end, "Exec=" + serviceExecutable);
            }
        }
        if(dbus_service_file.resize(0))
            dbus_service_file.write(contents.join("\n").toUtf8());
        else
        {
            qCritical() << "Error setting service executable: Could not write to the file " + _service_name + ".desktop.";
            return false;
        }
    }
    return true;
}

QString ApplicationInterface::get_service_executable(const QString &service_name)
{
    auto path_to_file = qEnvironmentVariable("HOME") +
                        "/.local/share/dbus-1/services/" +
                        service_name + ".service";

    auto file = QFile(path_to_file);

    if (not file.exists())
    {
        qCritical() << "Error getting service executable: " + path_to_file + " file does not exist.";
        return "";
    }

    if (not file.open(QIODevice::OpenModeFlag::ReadOnly | QIODevice::OpenModeFlag::ExistingOnly))
    {
        qCritical() << "Error getting service executable: Could not read from file " + path_to_file;
        return "";
    }

    QTextStream input(&file);

    auto contents = file_contents(input);

    for (const auto& line : contents)
    {
        if (line.startsWith("Exec="))
        {
            return line.sliced(QString("Exec=").length());
        }
    }

    return "";
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


