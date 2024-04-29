import QtQuick
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3


Window {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("ImageView")
    color: "#252525"
    minimumWidth: stack.implicitWidth
    minimumHeight: stack.implicitHeight + tabs.implicitHeight

    property var showErrorMessage: (message) =>  {
        error_show_animation.start()
        error_message.visible = true
        error_message.error_text = message
        showingError = ""
    }
    property string showingError : ""
    onShowingErrorChanged: {
        if (showingError.length > 0)
            showErrorMessage(showingError)
    }


    property string path_to_image: "empty"
    property string path_to_executable
    property string service_name
    property string mime_types
    property string errpr_message

    signal somethingDropped(smth : var)
    signal applyButtonClick(service : string, executable : string, mimetypes : string)

    TabBar
    {
        id: tabs
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: window.top
        currentIndex: 0

        background: Rectangle {
            color: "#252525"
        }

        TabButton {
            id: image_button
            width: window.width/2
            background: Rectangle
            {
                color: "#181818"
            }
            contentItem: Label
            {
                text: "ImageView"
                color: "white"
                anchors.verticalCenter: image_button.verticalCenter
            }
            OpacityAnimator on opacity
            {
                id: image_show_animation
                target: image_button
                from: 0.2
                to: 1
                duration: 1000
                running: false
            }

            OpacityAnimator on opacity
            {
                id: image_hide_animation
                target: image_button
                from: 1
                to: 0.2
                duration: 500
                running: false
            }

            onHoveredChanged:
            {
                if (checked)
                    return
                if (hovered)
                {
                    image_hide_animation.stop()
                    image_show_animation.start()
                }
                else
                {
                    image_show_animation.stop()
                    image_hide_animation.start()
                }
            }

            onCheckedChanged:
            {
                if(checked){
                    opacity = 1
                    return
                }
                image_show_animation.stop()
                image_hide_animation.start()
            }

        }

        TabButton {
            id: settings_button
            width: window.width/2
            opacity: 0.2
            checked: false
            background: Rectangle
            {
                color: "#181818"
            }
            contentItem: Label
            {
                text: "Settings"
                color: "white"
                anchors.verticalCenter: settings_button.verticalCenter
            }

            OpacityAnimator on opacity
            {
                id: settings_show_animation
                target: settings_button
                from: 0.2
                to: 1
                duration: 1000
                running: false
            }

            OpacityAnimator on opacity
            {
                id: settings_hide_animation
                target: settings_button
                from: 1
                to: 0.2
                duration: 500
                running: false
            }

            onHoveredChanged:
            {
                if (checked)
                    return
                if (hovered)
                {
                    settings_hide_animation.stop()
                    settings_show_animation.start()
                }
                else
                {
                    settings_show_animation.stop()
                    settings_hide_animation.start()
                }
            }

            onCheckedChanged:
            {
                if(checked){
                    opacity = 1
                    return
                }
                settings_show_animation.stop()
                settings_hide_animation.start()
            }
        }
    }

    StackLayout
    {
        property string text_dropped: "empty"
        id: stack
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: tabs.currentIndex
        anchors.top: tabs.bottom
        anchors.bottom: parent.bottom

        Item {
            id: image_tab
            visible: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Image {
                id: image_holder
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                visible: path_to_image != "empty"

                source:  path_to_image != "empty" ?  path_to_image : ""
            }

            Text {
                text: "Drop image to view it"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                visible: path_to_image == "empty"
            }

            DropArea
            {
                id: image_drop
                anchors.fill: image_tab
                visible: true
                onDropped: (drop) => somethingDropped(drop)
            }
        }
        Item {
            id: settings_tab
            width: 800
            height: 458
            visible: false
            anchors.bottomMargin: 60

            GridLayout {
                id: grid
                columns: 2
                anchors.fill: parent
                anchors.margins: 20
                anchors.bottomMargin: 60
                Label
                {
                    visible: true
                    text: "Service name (used for DBus Activation):"
                    color: "white"
                    Layout.alignment: Qt.AlignTop
                }

                TextField
                {
                    id: service_input
                    readOnly: true
                    implicitWidth: 250
                    text: service_name
                    background: Rectangle {
                        color: "#3c3b3b"
                        border.color: focus ? "lightblue" : "gray"
                        radius: 2
                    }
                    color: "#e46708"
                    Layout.alignment: Qt.AlignLeft
                }

                Label
                {
                    visible: true
                    text: "Path to program's executable:"
                    color: "white"
                    Layout.alignment: Qt.AlignTop
                }

                TextField
                {
                    id: executable_input
                    text: path_to_executable
                    readOnly: true
                    implicitWidth: 250
                    background: Rectangle {
                        color: "#3c3b3b"
                        border.color: focus ? "lightblue" : "gray"
                        radius: 2
                    }
                    color: "#e46708"
                    Layout.alignment: Qt.AlignLeft
                }

                Label
                {
                    visible: true
                    text: "Supported types (MIME, used for \"Open with other application\":"
                    color: "white"
                    Layout.alignment: Qt.AlignTop
                }
                ScrollView{
                    TextArea
                    {
                        id: mime_types_field
                        background: Rectangle {
                            color: "#3c3b3b"
                            implicitHeight: 200
                            Layout.minimumHeight: 200
                            implicitWidth: 250
                            border.color: focus ? "lightblue" : "gray"
                            radius: 2
                        }
                        text: mime_types
                        color: "#e46708"
                    }
                    Layout.minimumHeight: 200
                    Layout.minimumWidth: 250
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignLeft
                }

                Label
                {
                    Layout.alignment: Qt.AlignTop
                    text: "WARNING: Do not apply any changes, until you know, what are you actually doing.\n"+
                    "Changing any of those settings can cause program to stop working properly"
                    color: "white"
                }

                Button
                {
                    Layout.bottomMargin: 10
                    Layout.topMargin: 10
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.alignment: Qt.AlignLeft
                    id: ok_button
                    text: "Apply"
                    palette {
                        button: "#181818"
                    }
                    onClicked: {
                        applyButtonClick(service_input.text, executable_input.text, mime_types_field.text)
                    }
                }
            }
        }
    }
    Rectangle {
        id: error_message
        visible: false
        property int down_margin
        property string error_text : ""
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -down_margin
        color: "darkred"
        height: 50
        MouseArea{
            anchors.fill: parent
            onClicked: parent.visible = false
        }
        Label
        {
            id: error_label
            text: error_message.error_text + " (Click to hide)"
            color: "white"
            anchors.left: error_message.left
            anchors.verticalCenter: error_message.verticalCenter
            anchors.margins: 10
        }

        PropertyAnimation
        {
            id: error_show_animation
            target: error_message
            property: "down_margin"
            from: 50
            to: 0
        }

    }

}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.75}D{i:1}D{i:10;invisible:true}D{i:15}
}
##^##*/
