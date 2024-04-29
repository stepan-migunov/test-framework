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

    ///This DBus method called when user tries to launch application possible with desktop shortcut
    Q_NOREPLY void Activate(const QMap<QString, QVariant>& platform_data);

    ///This DBus method called when user tries to open file with this application
    Q_NOREPLY void Open(const QList<QString>& uris,
                                const QMap<QString, QVariant>& platform_data);

    ///This DBus method called when user tries to perform options, provided by Desktop Entry File.
    Q_NOREPLY void ActivateAction(const QString& action_name,
                                          const QList<QVariant> parameter,
                                          const QMap<QString, QVariant>& platform_data);
public:
    ApplicationInterface(QObject* object = nullptr);

    //D-BUS Activation functions
    void setActivate(activate_function_type activate) { m_activate = activate; }
    void setOpen(open_function_type open) { m_open = open; }
    void setActivateAction(activate_action_function_type activate_action) { m_activate_action = activate_action; }

    //Modifying .service file

    /// Modifies $HOME/.local/dbus-1/services/${service_name}.service file,
    /// by changin 'Exec=' key to $serviceExecutable].
    /// If no file found, tries to create one.
    /// Returns false, if creation or modification fails
    bool set_service_executable(const QString& serviceName, const QString& serviceExecutable);


    /// Does not modify $HOME/.local/dbus-1/services/${service_name}.service file.
    /// Returns "" if no file found
    /// Retruns "" if it can not read from file, or no 'Exec=' section found in file,
    /// returns anything after 'Exec=' key otherwise.
    QString get_service_executable(const QString& service_name);

    //Connecting to the DBus


    /// Does not modify $HOME/.local/dbus-1/services/${service_name}.service file,
    /// Called when application wants to register service in DBus daemon
    bool register_service(const QString& serviceName);

    /// Does not modify $HOME/.local/dbus-1/services/${service_name}.service file,
    /// Called when applicaiton wants to register object ${objectPath} on service ${service_name}
    /// If objectPath is not provided, ${service_name} is used, but every dot is replaced with slash
    bool registerObject(const QString& objectPath = "/", QObject* object = nullptr);



    virtual ~ApplicationInterface();
};

class MIME_DBUS_FRAMEWORK_EXPORT DesktopEntryEditor : public QObject
{
    Q_OBJECT

public:
    DesktopEntryEditor();
    explicit DesktopEntryEditor(QObject* parent);

    //Desktop Entry's Name

    /// Modifies $HOME/.local/applications/${service_name}.desktop file.
    /// Checks if this file exists.
    /// If it does not exist, returns false;
    /// If fails to modify it, returns false;
    /// If ${language} == [default], modifies 'Name=' key;
    /// Otherwise, modifies 'Name${language}=' key;
    /// Returns true if succeeded
    Q_INVOKABLE bool set_name(const QString& service_name, const QString& language, const QString& name); //TODO

    /// Does not modify $HOME/.local/applications/${service_name}.desktop file;
    /// Returns "" if no file found;
    /// Returns "" if ${service_name}.desktop file is not a valid Desktop Entry file
    /// Returns "" if no key 'Name${language}=' found
    /// Returns anything after 'Name${language}= key, if ${language} != [default], or
    /// Returns anything after 'Name=' key, if ${language} == [default]
    Q_INVOKABLE QString get_name(const QString& service_name, const QString& language);

    ///Does not modify $HOME/.local/applications/${service_name}.desktop file;
    /// Returns {} if no file found;
    /// Returns {} if ${service_name}.desktop file is not a valid Desktop Entry file
    /// Returns QMap, consising of pais <key, value>, where 'key' is in form [lang], and value is
    /// anything after 'Name[lang]=' key in ${service_name}.desktop in [Desktop Entry] section.
    /// Additionally this QMap contains pair <[default], default_value>, where
    /// default_value is anything after 'Name=' key in [Desktop Entry] section.
    Q_INVOKABLE QMap<QString, QString> get_names(const QString& service_name);


    //MimeTypes

    /// Checks if ${type} already added by calling get_types. If true, returns true
    /// Modifies $HOME/.local/applications/${service_name}.desktop file, by adding ${type} to section MimeType=
    /// Could create file, if nothing is provided
    /// Returns false, if could not write to .desktop file
    Q_INVOKABLE bool add_type(const QString& service_name, const QString& type);

    ///Does not mdodify $HOME/.local/applications/${service_name}.desktop file. Returns list of types
    /// under the key "MimeType=". Returns {} if no key provided or no file found.
    Q_INVOKABLE QList<QString> get_types(const QString& service_name);

    /// Checks if type exists by calling get_types
    /// Modifies $HOME/.local/applications/${service_name}.desktop file, by removing ${type} from section MimeType=
    /// returns true if succeded or if get_types does not contain ${type}. Otherwise returns false
    Q_INVOKABLE bool remove_type(const QString& service_name, const QString& type);


    //Desktop Entry executable

    ///Modifies $HOME/.local/applications/${service_name}.desktop file, by adding ${type} to key Exec=
    /// Creates tries to create file, if nothing was found.
    /// Returns false: if short service_name provided, invalid .desktop file found, or fails to create .desktop entry file
    /// Returns true if succeeded either modifying or creating file
    Q_INVOKABLE bool set_service_executable(const QString& service_name, const QString& executablePath);

    ///Does not modify $HOME/.local/applications/${service_name}.desktop file.
    /// Returns "" if: either no or invalid .desktop file found, or if fails to open/read from file
    /// Returns anything after Exec= key in [Desktop Entry] section.
    Q_INVOKABLE QString get_service_executable(const QString& service_name);


    //Action's names

    /// Checks if action exists by calling get_actions.
    /// Modifies $HOME/.local/applications/${service_name}.desktop file, by repacing anything after Name${language}= key
    /// in appropriate [Desktop Action ${action}] section. If ${language} == [default], modifies "Name=" key.
    /// Returns false if either no file found or invalid file provided
    Q_INVOKABLE bool set_action_name(const QString& service_name,
                                     const QString& action,
                                     const QString& language,
                                     const QString& action_name);
    /// Checks if action exists by calling get_acitons. If no action found, returns ""
    /// Does not modify $HOME/.local/applications/${service_name}.desktop file
    /// Returns "" if ${service_name}.desktop file does not exist, or fails to find Name${language}= key under
    /// action [Desktop Action ${action}].
    Q_INVOKABLE QString get_action_name(const QString& service_name,
                                        const QString& action,
                                        const QString& language);

    /// Checks if action exists by calling get_acitons. If no action found, returns ""
    /// Does not modify $HOME/.local/applications/${service_name}.desktop file
    /// Scanning first section [Desktop Action ${action}],
    /// find lines satisfying template Name[*]= or Name=
    /// First type is put to map in a way <[lang], <anithing after '='>>
    /// Second type is put to map under key '[default]'
    /// Returns a map with keys of kind [language] and names.
    /// Returns {} if no action found
    Q_INVOKABLE QMap<QString, QString> get_action_names(const QString& service_name,
                                                        const QString& action);

    /// Checks if action exists by calling get_actions
    /// Returns false if there is no such action, or ${language} == [default]
    /// Checks if action's name exists by calling get_action_names,
    /// returns true if no name for this lang is found.
    /// Modifies $HOME/.local/applications/${service_name}.desktop file,
    /// by removing line 'Name${language}=*' under section '[Desktop Action ${action}]'
    /// Returns either true if success, or false if modifying file fails.
    Q_INVOKABLE bool remove_action_name(const QString& service_name,
                                        const QString& action,
                                        const QString& language,
                                        const QString& action_name);


    //Action's executable

    /// Checks if action ${action} exists, returns "" if does not
    /// Does not modify $HOME/.local/applications/${service_name}.desktop file.
    /// Returns false if no file found, or if fails to open it
    /// Returns anything after 'Exec=' key under '[Desktop Action ${action}]' section
    Q_INVOKABLE QString get_action_exec(const QString& service_name,
                                     const QString& action);

    /// Checks if action ${action} exists, returns false if does not
    /// Modifies $HOME/.local/applications/${service_name}.desktop file.
    /// Returns false if it is not valid .desktop file, or
    Q_INVOKABLE bool set_action_exec(const QString& service_name,
                                        const QString& action,
                                        const QString& action_exec);


    //Getting/adding/removing actions

    /// Does not modify Does not modify $HOME/.local/applications/${service_name}.desktop file;
    /// Returns {} if no file found;
    /// Returns {} if ${service_name}.desktop file is not a valid Desktop Entry file
    /// Takes anything after 'Actions=' key, and splits it by semicolon
    Q_INVOKABLE QStringList get_actions(const QString& service_name);

    /// Checks if action ${action} already exists. If it is, returns false;
    /// Modifies $HOME/.local/applications/${service_name}.desktop file;
    /// Returns false if no file found;
    /// Returns false if ${service_name}.desktop file is not a valid Desktop Entry file;
    /// Appending '${action};' to the 'Actions=' key
    /// Appending
    /// [Desktop Action ${action}]
    /// Name=${action_name}
    /// Exec=${action_exec}
    /// Icon=${action_icon}
    ///
    /// Adding icon is optional, and does not appending, if ${action_icon} == ""
    /// Returns true if modifying succeeded, otherwise returns false
    Q_INVOKABLE bool add_action(const QString& service_name,
                                const QString& action,
                                const QString& action_name,
                                const QString& action_exec,
                                const QString& action_icon = "");

    /// Checks if action ${action} exists. If it is not, returns false;
    /// Modifies $HOME/.local/applications/${service_name}.desktop file;
    /// Returns false if no file found;
    /// Returns false if ${service_name}.desktop file is not a valid Desktop Entry file;
    /// Removes ${action} from 'Actions=' key in [Desktop Entry] section
    /// Removes lines starting from '[Desktop Action ${action}]', and ending by last string
    /// in file, which does not start with '[' symbol .
    /// Returns true if modifying succeeded, and false otherwise
    Q_INVOKABLE bool remove_action(const QString& service_name,
                                   const QString& action);

};

}

#endif // MIME_DBUS_FRAMEWORK_H
