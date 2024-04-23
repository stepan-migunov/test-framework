#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <mime_dbus_framework.h>
#include <iostream>


int main(int argc,  char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    MyFramework::MimeDBusFramework f;
    MyFramework::ApplicationInterface i;

    auto service_name = QString("org.example.myImageViewer");
    auto object_path = QString("/org/example/myImageViewer");

    auto executable_path = QString("/usr/bin/image_viewer");

    const QUrl main_url(u"qrc:/image_viewer/main.qml"_qs);\
    const QUrl image_preview(u"qrc:/image_viewer/image_view.qml"_qs);

    f.setServiceName(service_name);
    f.add_type("image/jpeg", executable_path);

    i.register_service(service_name, executable_path);
    i.registerObject(object_path);
    i.setActivate(
                [&engine, &main_url](...)
                {
                    engine.load(main_url);
                }
    );

    i.setOpen(
                [&engine, &image_preview](const QList<QString>& uris, ...)
                {
                    for (const auto& file: uris){
                        QString _file = "file:";
                        if(file.startsWith("file:/"))
                        {
                            _file = file;
                        }
                        else
                        {
                            _file.append(file);
                        }
                        engine.setInitialProperties({{"image_path", _file}});
                        engine.load(image_preview);
                    }
                }
    );

    return app.exec();
}
