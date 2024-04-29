#include "settingsprovider.h"

#include <QFile>
#include <QMimeDatabase>

SettingsProvider::SettingsProvider(QQmlApplicationEngine& eng,
                                   const QString& service_name,
                                   MyFramework::DesktopEntryEditor& framework,
                                   MyFramework::ApplicationInterface& interface,
                                   const QUrl& url,
                                   QObject *parent)
    : QObject(parent),
      engine(eng),
      frame(framework),
      interface(interface),
      main_url(url),
      actual_service_name(service_name)
{

}

void SettingsProvider::provide_file_settings(QObject *obj, const QUrl &url)
{
    Q_UNUSED(url)
    obj->setProperty("path_to_executable", frame.get_service_executable(actual_service_name));
    obj->setProperty("service_name", actual_service_name);
    obj->setProperty("mime_types", frame.get_types(actual_service_name).join('\n'));
}

void SettingsProvider::accept_settings(const QString &service, const QString &executable, const QString &mime_types)
{
    auto s = sender();


    QStringList existing_types = frame.get_types(actual_service_name);
    existing_types.removeDuplicates();

    QStringList provided_types = mime_types.split('\n',Qt::SkipEmptyParts);
    provided_types.removeDuplicates();

    QVector<bool> new_types(provided_types.size());

    for (qsizetype i = 0; i < provided_types.size(); ++i)
        new_types[i] = not existing_types.contains(provided_types[i]);
    for (qsizetype i = 0; i < provided_types.size(); ++i)
        if(new_types[i])
        {
            QMimeDatabase db;
            auto type = db.mimeTypeForName(provided_types[i]);
            if (not type.isValid())
            {
                s->setProperty("showingError", provided_types[i] + QString(" is not a valid MimeType"));
                continue;
            }

            auto adding = frame.add_type(actual_service_name, provided_types[i]);
            if (not adding)
            {
                s->setProperty("showingError", QString("Error occured while adding type ") + provided_types[i]);
            }
        }



    QVector<bool> removing_types(existing_types.size());
    for (qsizetype i = 0; i < existing_types.size(); ++i)
        removing_types[i] = not provided_types.contains(existing_types[i]);

    for (qsizetype i = 0; i < existing_types.size(); ++i)
        if(removing_types[i])
        {
            auto removing = frame.remove_type(actual_service_name, existing_types[i]);
            if (not removing)
            {
                s->setProperty("showingError", QString("Error occured while removing type ") + provided_types[i]);
            }
        }

    provide_file_settings(s, QUrl());
}

void SettingsProvider::accept_drop(const QVariant &drop)
{
    QObject* s = sender();
    QVariant _drop(drop);
    if (drop.canConvert<const QObject*>())
        _drop.convert(QMetaType::fromType<QObject*>());
    else
        return;

    auto data = _drop.value<QObject*>();

    if (not data->property("hasUrls").value<bool>())
        return;

    auto uris = data->property("urls").value<QList<QUrl>>();

    bool first_valid_uri_used = false;

    for (const auto& uri : uris)
    {
        if (not uri.isLocalFile()){
            s->setProperty("showingError", "No file found");
            continue;
        }

        QFile image(uri.toLocalFile());
        if (not image.exists()){
            s->setProperty("showingError", QString("File ") + uri.toLocalFile() + QString(" does not exist."));
            continue;
        }

        QMimeDatabase db;

        auto type = db.mimeTypeForFile(uri.toLocalFile()).name();
        auto supported_types = frame.get_types(actual_service_name);
        if (not supported_types.contains(type))
        {
            s->setProperty("showingError", "Unsupportable type of file");
            continue;
        }

        if(not first_valid_uri_used)
        {
            s->setProperty("path_to_image", uri);
            first_valid_uri_used = true;
        }
        else
        {
            engine.setInitialProperties({{"path_to_image", uri}});
            engine.load(main_url);
        }
    }
}


