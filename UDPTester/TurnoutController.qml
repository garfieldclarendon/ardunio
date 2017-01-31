import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import Utils 1.0

Rectangle {
    id: outerRect
    property int currentControllerID: 0

    TurnoutModel {
        id: turnoutModel
        controllerID: currentControllerID
    }

    radius: 20
    border.color: "grey"
    Component {
        id: turnoutDelegate
         GroupBox {
            id: wraper
            width: outerRect.width - ui.applyRatio(35)
            height: (activate.height * 2) + ui.applyRatio(25) + (buttonLayout.spacing * 2) + (ui.margin * 2)
//            title: itemName
//            clip: true
//            border.color: "grey"
//            border.width: 1
//            color: id > 0 ? "white" : "lightgrey"
            function getStateText(state)
            {
                if(state === "0" || state === "?")
                    return "Unknown";
                else if(state === "1")
                    return "Normal";
                else if(state === "2")
                    return "ToDiverging";
                else if(state === "3")
                    return "Diverging";
                else if(state === "4")
                    return "ToNormal";
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.debug("Mouse Clicked!")
                    listView.currentIndex = index
                }
            }
            ColumnLayout {
                spacing: ui.margin
                anchors.fill: parent
                anchors.margins: ui.margin
                RowLayout {
                    Layout.fillWidth: true
                    Text
                    {
                        id: controllerNameText
                        text: id > 0 ? itemName : ""
                        font.bold: true
                        font.pointSize: ui.applyFontRatio(ui.baseFontSize + 4)
                        anchors.margins: ui.margin
                        Layout.minimumWidth:  ui.applyRatio(50)
                        color: "blue"
                    }
                    ColumnLayout {
                        Layout.minimumWidth: ui.applyRatio(100)
                        Text {
                            text: id > 0 ? '<b>Description:</b> ' + itemDescription : ""
                            verticalAlignment: Text.AlignVCenter
                            height: parent.height
                         }
                        Text {
                            text: id > 0 ? '<b>ID:</b> ' + id : ""
                            verticalAlignment: Text.AlignVCenter
                            height: parent.height
                         }
                        Text {
                            text: id > 0 ? '<b>State:</b> ' + wraper.getStateText(currentState) : ""
                            verticalAlignment: Text.AlignVCenter
                            height: parent.height
                         }
                    }
                    ColumnLayout {
                        id: buttonLayout
                        Rectangle {
                            id: buffer1
                            Layout.fillHeight: true
                        }
                        Button {
                            id: activate
                            text: "Activate"
                            enabled: true
                            onClicked: {
                                console.debug("current state: " + currentState);
                                broadcaster.sendMessage(102, currentControllerID, 1, id, currentState === "1" ? 3 : 1, 0);
                            }
                        }
                        Button {
                            id: actionButton
                            text: id > 0 ? "Remove" : "Add"
                            onClicked: {
                                console.debug("Action button clicked");
                            }
                        }
                        Rectangle {
                            id: buffer2
                            Layout.fillHeight: true
                        }
                    }
                }
            }
        }
    }

    ListView {
        anchors.fill: parent
        anchors.margins: ui.applyRatio(15)
        id: listView
        model: turnoutModel
        delegate: turnoutDelegate
        spacing: ui.applyRatio(2)
        focus: true
        clip: true
    }
}

