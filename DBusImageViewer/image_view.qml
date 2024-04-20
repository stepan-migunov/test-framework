import QtQuick 2.0

Window {
    id: image_view
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    property string image_path
    Image {
        id: image
        anchors.fill: parent
        source: image_path
    }

}
