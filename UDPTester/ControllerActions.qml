import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import Utils 1.0

Item {
    property int controllerID: 0
    signal editClicked()

    width: resetButton.width * 4 + 15
    height: resetButton.height + 15

    Rectangle {
        anchors.fill: parent
        color: "lightgrey"
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
                    console.debug("Reset Button Clicked!!");
                    editClicked();
                }
            }
            Button {
                id: resetButton
                text: "Reset"
                onClicked: {
                    console.debug("Reset Button Clicked!!");
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
                id: updateFirmware
                text: "Update Firmware"
                onClicked: {
                    console.debug("Update Firmware Button Clicked!!");
                    broadcaster.sendDownloadFirmware(controllerID);
                }
            }
        }
    }
}
