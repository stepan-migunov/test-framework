import QtQuick 2.0

Window {
    width: 640
    height: 480
    visible: true
    property string image_path
    Image
    {
        source: image_path
        anchors.fill: parent
    }
}
