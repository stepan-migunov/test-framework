#include "applicationInterface.h"


applicationInterface::applicationInterface(QObject *object): QDBusAbstractAdaptor(object)
{

}

applicationInterface::~applicationInterface(){}

void applicationInterface::Activate(const QMap<QString, QVariant> &platform_data)
{
    Q_UNUSED(platform_data)
    const QUrl url(u"qrc:/DBusImageViewer/main.qml"_qs);
    engine->load(url);
}

void applicationInterface::Open(const QList<QString> &uris, const QMap<QString, QVariant> &platform_data)
{

    Q_UNUSED(platform_data)
    QFile check_image(uris[0]);
    engine->setInitialProperties({
                                     {"image_path",
                                      check_image.exists()
                                      ?
                                      "file:" + check_image.fileName()
                                      :
                                      ""
                                     }
                                 });
    engine->load(QUrl(u"qrc:/DBusImageViewer/image_view.qml"_qs));
}

void applicationInterface::ActivateAction(const QString &action_name, const QList<QVariant> parameter, const QMap<QString, QVariant> &platform_data)
{
    Q_UNUSED(action_name)
    Q_UNUSED(parameter)
    Q_UNUSED(platform_data)
}
