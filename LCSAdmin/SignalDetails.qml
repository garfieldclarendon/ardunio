import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.1
import Utils 1.0

Rectangle {
    id: main
    property var deviceEntity

    signal saveClicked();
    signal cancelClicked();
    signal aspectDoubleClicked(int aspectID);

    height: 100
    width: 100

    function saveData()
    {
        console.debug("SignalDetails::saveData!!!!");
        aspectModel.save();
    }
    MessageDialog {
        id: messageDialog
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No
        title: "New Signal Aspect"
        text: "The new Signal Aspect needs to be saved before adding conditions.  Save now?"
        onYes: {
            saveData();
            messageDialog.visible = false;
        }
    }

    SignalAspectModel {
        id: aspectModel
        deviceID: deviceEntity.data ? deviceEntity.data.deviceID : 0
    }

    ListModel {
        id: pinModel
        ListElement { text: "Off"; mode: 0 }
        ListElement { text: "On"; mode: 1 }
        ListElement { text: "Flashing"; mode: 4 }
    }

    GridLayout {
        anchors.fill: parent
        columns: 3
        GenericDeviceDetails {
            devicePropertiesVisible: false
            deviceEntity: main.deviceEntity
            Layout.columnSpan: 3
            Layout.fillWidth: true
            onSaveClicked: {
                main.saveData();
                main.saveClicked();
            }
            onCancelClicked: {
                main.cancelClicked();
            }
        }
        Item {
            width: addButton.width
            height: addButton.height
        }
        Label {
            Layout.fillWidth:  true
            text: "<b>Signal Aspects</b>"
            color: "blue"
            font.pixelSize: ui.baseFontSize + 10
            horizontalAlignment: Qt.AlignHCenter
        }
        ToolButton {
            id: addButton
            contentItem: Image {
                source: "Images/add.png"
                sourceSize.width: 18
                sourceSize.height: 18
            }
            onClicked: {
                var entity = aspectModel.getEntity(-1);
                aspectModel.setEntity(-1, entity);
                aspectView.currentIndex = aspectModel.getRowCount() - 1
            }
        }

        ListView {
            id: aspectView
            clip: true
            model: aspectModel
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
            delegate: Rectangle {
                border.color: "lightgrey"
                border.width: 1
                width: parent.width
                height: redCombo.height + 10

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: {
                        if(signalAspectID > 0)
                            aspectDoubleClicked(signalAspectID);
                        else
                            messageDialog.visible = true;
                    }
                }

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    columns: 9
                    Label {
                        text: "<b>Green: </b>"
                    }
                    ComboBox {
                        id: greenCombo
                        model: pinModel
                        textRole: "text"
                        currentIndex: greenMode
                        onActivated: {
                            greenMode = currentIndex;
                        }
                     }
                    Label {
                        text: "<b>Yellow: </b>"
                    }
                    ComboBox {
                        id: yellowCombo
                        model: pinModel
                        textRole: "text"
                        currentIndex: yellowMode
                        onActivated: {
                            yellowMode = currentIndex;
                        }
                     }
                    Label {
                        text: "<b>Red: </b>"
                    }
                    ComboBox {
                        id: redCombo
                        model: pinModel
                        textRole: "text"
                        currentIndex: redMode
                        onActivated: {
                            redMode = currentIndex;
                        }
                     }
                    ToolButton {
                        id: deleteButton
                        contentItem: Image {
                            source: "Images/delete.png"
                            sourceSize.width: 18
                            sourceSize.height: 18
                        }
                        onClicked: {
                            aspectModel.deleteRow(index);
                        }
                    }
                    ToolButton {
                        id: upButton
                        enabled: index > 0
                        contentItem: Image {
                            source: "Images/arrow_up.png"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            layer.enabled: true
                            layer.effect: Desaturate {
                                desaturation: parent.enabled ? 0 : 1
                                Behavior on desaturation { NumberAnimation { easing.type: Easing.InOutQuad } }
                            }
                        }
                        onClicked: {
                            aspectModel.moveUp(index);
                        }
                    }
                    ToolButton {
                        id: downButton
                        enabled: index < aspectModel.rowCount - 1
                        contentItem: Image {
                            source: "Images/arrow_down.png"
                            sourceSize.width: 18
                            sourceSize.height: 18
                            layer.enabled: true
                            layer.effect: Desaturate {
                                desaturation: parent.enabled ? 0 : 1
                                Behavior on desaturation { NumberAnimation { easing.type: Easing.InOutQuad } }
                            }
                        }
                        onClicked: {
                            aspectModel.moveDown(index);
                        }
                    }
                }
            }
        } //ListView

    }
}
