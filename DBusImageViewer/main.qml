import QtQuick

Window {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    Text {
        id: text_viewer

        text: ("This is an example of MimeFramework")
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
