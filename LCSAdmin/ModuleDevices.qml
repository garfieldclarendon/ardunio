import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Utils 1.0

GroupBox {
    id: deviceModuleGroup
    title: "<b>Module Devices</b>"
    property int moduleID: -1
    property int moduleClass: 0

    TextMetrics {
        id: comboMetrics
        text: "CONTROLLER NAME"
    }

    DeviceModuleModel {
        id: model
        moduleID:   deviceModuleGroup.moduleID
    }

    DeviceModel {
        id: deviceModel
        deviceClass: {
            if(moduleClass == 1)
                return 1;
            else
                return 0;
        }
    }

    GridLayout {
        anchors.fill: parent
        columns: 2
        Item {
            height: addButton.height
            Layout.fillWidth: true
        }
        ToolButton {
            id: addButton
            contentItem: Image {
                source: "Images/add.png"
                sourceSize.width: 18
                sourceSize.height: 18
            }
            onClicked: {
//                var entity = model.getEntity(-1);
//                model.setEntity(-1, entity);
//                modulesList.currentIndex = modulesList.rowCount - 1
            }
        }
        ListView {
            id: modulesList
            clip: true
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            Component {
                id: detialDelegate
                RowLayout {
                    id: wrapper
                    width: parent.width
                    height: deviceIDCombo.height + 12
                    Text {
                        id: moduleIDLabel
                        text: "<b>ID:</b>"
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: deviceIDCombo
                        model: deviceModel
                        textRole: "deviceName"
                        currentIndex: deviceModel.getDeviceRow(deviceID);
                        Layout.minimumWidth: comboMetrics.width
                        Layout.fillWidth: true
                        onActivated: {
                            deviceID = deviceModel.data(deviceIDCombo.currentIndex, "deviceID");
                        }
                     }
                    Text {
                        id: portLabel
                        text: "<b>Port:</b>"
                        horizontalAlignment: Text.AlignRight
                    }
                    TextField {
                        id: portText
                        text: port ? port : ""
                        Layout.maximumWidth: 60
                        onTextChanged: {
                            port = portText.text
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
                            model.deleteRow(index);
                        }
                    }
                }
            }

            model: model
            delegate: detialDelegate
            focus: true
        }
    }

    function saveData()
    {
        model.save();
    }
}
