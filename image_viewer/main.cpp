#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <mime_dbus_framework.h>
#include <settingsprovider.h>
#include <iostream>
#include <QFile>
#include <QVariant>
#include <QMimeDatabase>



int main(int argc,  char *argv[])
{

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    const auto service_name     = QString("org.example.myImageViewer");

    const auto object_path      = QString("/org/example/myImageViewer");

    const auto executable_path  = QCoreApplication::applicationFilePath() + QString(" %F");

    const auto main_url         = QUrl(u"qrc:/image_viewer/main.qml"_qs);

    MyFramework::DesktopEntryEditor f;
    MyFramework::ApplicationInterface i;
    SettingsProvider s(engine, service_name, f, i,  main_url, &app);

    QObject::connect(&engine,&QQmlApplicationEngine::objectCreated, &app, [&s](QObject* obj, const QUrl &url){
        Q_UNUSED(url)
        QObject::connect(obj,SIGNAL(somethingDropped(QVariant)), &s, SLOT(accept_drop(QVariant)));
        QObject::connect(obj, SIGNAL(applyButtonClick(QString, QString, QString)), &s, SLOT(accept_settings(const QString&, const QString&, const QString&)));
    });
    QObject::connect(&engine,&QQmlApplicationEngine::objectCreated, &s, &SettingsProvider::provide_file_settings);

    f.set_service_executable(service_name, executable_path);

    i.register_service(service_name);
    i.set_service_executable(service_name, executable_path);
    i.registerObject(object_path);

    i.setActivate([&engine, &main_url](...)
                    {
                        engine.load(main_url);
                    });

    i.setOpen([&engine, &main_url, &f, &service_name](const QList<QString>& uris, const QMap<QString, QVariant>& platform_data)
                {
                    for (const auto& file: uris){
                        QString _file = "file:";
                        if(file.startsWith("file:/"))
                            _file = file;
                        else
                            _file.append(file);
                        QUrl uri(_file);
                        if (not uri.isLocalFile())
                            continue;

                        QFile image(uri.toLocalFile());
                        if (not image.exists())
                            continue;

                        QMimeDatabase db;

                        auto type = db.mimeTypeForFile(uri.toLocalFile()).name();
                        auto supported_types = f.get_types(service_name);
                        if (not supported_types.contains(type))
                            continue;


                        engine.setInitialProperties({{"path_to_image", _file}});
                        engine.load(main_url);
                    }
                });

    if (argc < 2)
        engine.load(main_url);
    return app.exec();
}
