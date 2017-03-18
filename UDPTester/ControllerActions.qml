import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import Utils 1.0

Item {
    property int controllerID: 0
    property int serialNumber: 0
    property bool enableAddModule: false
    property bool enableAddDevice: false
    signal editClicked()
    signal addModuleClicked()
    signal addDeviceClicked()

    implicitWidth: resetButton.width + editButton.width + sendConfig.width + updateFirmware.width + ui.applyRatio(20)
    implicitHeight: resetButton.height + ui.applyRatio(15)

    onControllerIDChanged: {
        console.debug("onControllerIDChanged:  New ID: " + controllerID);
    }

    onSerialNumberChanged: {
        console.debug("onSerialNumberChanged: " + serialNumber);
    }

    SystemPalette {
        id: systemPalette
    }
    Rectangle {
        anchors.fill: parent
        color: systemPalette.window
        border.color: "grey"
        border.width: 1
        radius: ui.applyRatio(10)
        RowLayout {
            id: buttonLayout
            anchors.centerIn: parent
            spacing: ui.margin
            Button {
                id: editButton
                text: "Edit"
                onClicked: {
                    console.debug("Edit Button Clicked!!");
                    editClicked();
                }
            }
            Button {
                id: resetButton
                text: "Restart"
                onClicked: {
                    console.debug("Restart Button Clicked!!");
                    broadcaster.sendResetCommand(controllerID);
                }
            }
            Button {
                id: sendConfig
                text: "Send Config"
                onClicked: {
                    console.debug("Send Config Button Clicked!!");
                    broadcaster.sendConfigData(controllerID);
                }
            }
            Button {
                id: resetConfig
                text: "Reset Config"
                onClicked: {
                    console.debug("Reset Config Button Clicked!! Serial Number: " + serialNumber);
                    broadcaster.sendMessage(16, controllerID, 0, 0, serialNumber, 0, 0, 0, 0);
                }
            }
            Button {
                id: updateFirmware
                text: "Update Firmware"
                onClicked: {
                    console.debug("Update Firmware Button Clicked!!");
                    broadcaster.sendDownloadFirmware(controllerID);
                }
            }
            Button {
                id: addModule
                text: "Add Module"
                enabled: enableAddModule
                onClicked: {
                    console.debug("Add Module Button Clicked!!");
                    addModuleClicked();
                }
            }
            Button {
                id: addDevice
                text: "Add Device"
                enabled: enableAddDevice
                onClicked: {
                    console.debug("Add Device Button Clicked!!");
                    addDeviceClicked();
                }
            }
        }
    }
}
