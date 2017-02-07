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

        GroupBox {
            id: wraper
            clip: true
            width: outerRect.width - ui.applyRatio(25)
            height: ui.applyRatio(100)

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.debug("Mouse Clicked!")
//                    stackView.item.currentIndex = index;
                }
                onDoubleClicked: {
                    showPanelModuleDetails(id);
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
                        text: id > 0 ? moduleIndex + " " + moduleName : index
                        font.bold: true
                        font.pointSize: ui.applyFontRatio(ui.baseFontSize + 4)
                        anchors.margins: ui.margin
                        Layout.minimumWidth: ui.applyRatio(175)
                        color: "blue"
                    }
                    ColumnLayout {
                        Layout.minimumWidth: ui.applyRatio(150)
                        Text {
                            text: id > 0 ? '<b>Index (Address):</b> ' + moduleIndex : ""
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

    function showPanelModuleDetails(id)
    {
        stackView.push({item:"qrc:/PanelModuleDetails.qml", properties:{panelID:id}})
//        stackView.push({item:"qrc:/EditController.qml"})
    }

    StackView {
        id: stackView
        clip:true
        anchors.fill: parent

        Component {
            id: list

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

        Component.onCompleted: {
            stackView.push(list);
        }
    }
}
