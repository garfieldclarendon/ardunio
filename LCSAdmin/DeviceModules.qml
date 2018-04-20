import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Utils 1.0

Rectangle {
    id: deviceModuleGroup
    implicitHeight: expanded ? (textMetrics.itemHeight * (modulesList.count + 1)) + 15 : textMetrics.itemHeight
    border.color: "lightgrey"
    border.width: 1
    property int deviceID: -1
    property int deviceClass: 0
    property bool expanded: true

    function saveData()
    {
        model.save();
    }

    TextMetrics {
        id: textMetrics
        property int itemHeight: Math.max(addButton.height, textMetrics.height + 35)
        text: "CLASS>MULTI-CONTROLLER"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            expanded = !expanded;
        }
    }

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
    Text {
        id: titleText
        text: modulesList.count > 0 ? "Assigned Controller Modules (" + modulesList.count + ")" : "Assigned Controller Modules"
        height: Math.max(addButton.height, implicitHeight)
        font.pixelSize: textMetrics.height + (textMetrics.height * 0.5)
        font.bold: true
        horizontalAlignment: Qt.AlignHCenter
        anchors.topMargin: 5
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
    ToolButton {
        id: addButton
        visible: expanded
        anchors.right: parent.right
        anchors.top: parent.top

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

    Item {
        id: containerItem
        anchors.left: parent.left
        anchors.top: titleText.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        opacity: expanded ? 1 : 0
            ListView {
                id: modulesList
                clip: true
                anchors.fill: parent
                anchors.margins: 5
                Component {
                    id: detialDelegate
                    RowLayout {
                        id: wrapper
                        width: parent.width
                        height: textMetrics.itemHeight
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
}
