import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Utils 1.0

GroupBox {
    id: deviceModuleGroup
    title: "<b>Assigned Controller Modules</b>"
    property int deviceID: -1
    property int deviceClass: 0

    DeviceModuleModel {
        id: model
        deviceID:  deviceModuleGroup.deviceID
    }

    ControllerModuleModel {
        id: moduleModel
        moduleClass: {
            if(deviceClass === 1 || deviceClass === 5)
                return 1;
            else if(deviceClass === 2 || deviceClass === 6)
                return 8;
            else if(deviceClass === 3 || deviceClass === 4)
                return 9;
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
                var entity = model.getEntity(-1);
                model.setEntity(-1, entity);
                modulesList.currentIndex = modulesList.rowCount - 1
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
                    height: portLabel.height + 12
                    Text {
                        id: moduleIDLabel
                        text: "<b>ID:</b>"
                        horizontalAlignment: Text.AlignRight
                    }
                    ComboBox {
                        id: moduleIDCombo
                        model: moduleModel
                        textRole: "moduleName"
                        currentIndex: moduleModel.getModuleRow(controllerModuleID);
                        Layout.minimumWidth: comboMetrics.width
                        Layout.fillWidth: true
                        onActivated: {
                            controllerModuleID = moduleModel.data(moduleIDCombo.currentIndex, "controllerModuleID");
                        }
                     }

//                    TextField {
//                        id: moduleIDText
//                        text: controllerModuleID ? controllerModuleID : ""
//                        onTextChanged: {
//                            controllerModuleID = moduleIDText.text
//                        }
//                    }
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
                            modulesList.model.deleteRow(index);
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
