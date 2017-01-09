import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Rectangle {
    id: outerRect
    property int currentControllerID: 0

    PanelModuleModel {
        id: panelModel
        controllerID: currentControllerID
    }

    Component {
        id: panelDelegate

        Item {
            id: wraper
            clip: true
            width: outerRect.width - ui.applyRatio(25)
            height: ui.applyRatio(100)
//            border.color: "grey"
//            border.width: 1
//            color: id > 0 ? "white" : "lightgrey"

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
                    Text
                    {
                        id: panelNameText
                        text: id > 0 ? panelIndex + " " + panelName : index
                        font.bold: true
                        font.pointSize: ui.applyFontRatio(6)
                        anchors.margins: ui.margin
                        Layout.minimumWidth: ui.applyRatio(175)
                        color: "blue"
                    }
                    ColumnLayout {
                        Layout.minimumWidth: ui.applyRatio(150)
                        Text {
                            text: id > 0 ? '<b>Index (Address):</b> ' + panelIndex : ""
                            verticalAlignment: Text.AlignVCenter
                            height: parent.height
                         }
                        Text {
                            text: id > 0 ? '<b>ID:</b> ' + id : ""
                            verticalAlignment: Text.AlignVCenter
                            height: parent.height
                         }
                    }
                    ColumnLayout {
                        Button {
                            id: editButton
                            text: "Edit"
                            enabled: id > 0 ? true : false
                            onClicked: {
                                console.debug("Edit button clicked");
                            }
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
    }

    ListView {
        anchors.fill: parent
        anchors.margins: ui.margin
        id: listView
        model: panelModel
        delegate: panelDelegate
        spacing: ui.applyRatio(2)
        focus: true
    }
}
