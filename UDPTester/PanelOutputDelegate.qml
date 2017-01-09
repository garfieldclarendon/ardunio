import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Component {
    Rectangle {
        id: wraper
        clip: true
        width: outerRect.width - 25
        height: 60
        border.color: "grey"
        border.width: 1
        color: id > 0 ? "white" : "lightgrey"

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.debug("Mouse Clicked!")
                listView.currentIndex = index
            }
        }
        ColumnLayout {
            spacing: 5
            anchors.fill: parent
            anchors.margins: 5
            RowLayout {
                Text
                {
                    id: nameText
                    text: id > 0 ? outputName : ""
                    font.bold: true
                    font.pointSize: 10
                    anchors.margins: 5
                    Layout.minimumWidth:  100
                    color: "blue"
                }
                Text {
                    text: id > 0 ? '<b>ID:</b> ' + id : ""
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                 }
                Text {
                    text: id > 0 ? '<b>Item:</b> ' + itemID : ""
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                 }
                Text {
                    text: id > 0 ? '<b>Type:</b> ' + itemType : ""
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                 }
                Text {
                    text: id > 0 ? '<b>On Value:</b> ' + onValue : ""
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                 }
                Text {
                    text: id > 0 ? '<b>Flashing Value:</b> ' + flashingValue : ""
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                 }
                Button {
                    id: actionButton
                    text: id > 0 ? "Remove" : "Add"
                    onClicked: {
                        console.debug("Action button clicked");
                    }
                }
            }
        }
    }
}

