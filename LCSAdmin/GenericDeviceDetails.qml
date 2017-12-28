import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

Rectangle {
    property var deviceEntity

    signal saveClicked();
    signal cancelClicked();

    implicitHeight: saveButton.height * 8
    width: 400

//    border.color: "red"
//    border.width: 1
    clip: true

    TextMetrics {
        id: comboMetrics
        text: "CONTROLLER NAME"
    }
/*
    DeviceUnknown,
    DeviceTurnout,
    DevicePanelInput,
    DevicePanelOutput,
    DeviceSignal = 4,
    DeviceSemaphore = 5,
    DeviceBlock = 6,
    DevicePanel = 7 // For old versions--no longer used
*/
    ListModel {
        id: classModel
        ListElement { text: "Unknown"; classID: 0 }
        ListElement { text: "Turnout"; classID: 1 }
        ListElement { text: "Panel Input"; classID: 2 }
        ListElement { text: "Panel Output"; classID: 3 }
        ListElement { text: "Signal"; classID: 4 }
        ListElement { text: "Semaphore"; classID: 5 }
        ListElement { text: "Block"; classID: 6 }
        ListElement { text: "Panel (do not use)"; classID: 7}
    }

    function getClassRow(classCode)
    {
        for(var x = 0; x < classModel.count; x++)
        {
            if(classModel.get(x).classID == classCode)
            {
                return x;
            }
        }
        return -1;
    }

    function saveData()
    {
        deviceEntity.setValue("deviceName", nameEdit.text);
        deviceEntity.setValue("deviceDescription", descriptionEdit.text);
        deviceEntity.setValue("port", portEdit.text);
    }

    function getClassName(classID)
    {
        if(classID === 0)
            return "Unknown";
        else if(classID === "1")
            return "Turnout";
        else if(classID === "2")
            return "Panel";
        else if(classID === "3")
            return "Route";
        else if(classID === "4")
            return "Signal";
        else if(classID === "5")
            return "Semaphore";
        else if(classID === "6")
            return "Block";
        else if(classID === "7")
            return "Multi-Controller";
        else if(classID === "8")
            return "System";
        else if(classID === "9")
            return "Application";

        return classID;
    }

    GridLayout {
        id: gridLayout
        columns: 3
        columnSpacing: 5
        anchors.fill: parent
        anchors.margins: 2
// First Row
        Label {
            id: deviceLabel
            text: "ID:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        Label {
            id: deviceIDEdit
            text: deviceEntity.data.deviceID
            Layout.fillWidth: true
        }
        Button {
            id: saveButton
            text: "Save"
            onClicked: {
                saveData();
                saveClicked();
                deviceProperties.saveData();
            }
        }
// Second Row
        Label {
            text: "Name:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        TextField {
            id: nameEdit
            text: deviceEntity.data.deviceName
            Layout.fillWidth: true
        }
        Button {
            id: cancelButton
            text: "Cancel"
            onClicked: {
                cancelClicked();
            }
        }
// Third Row
        Label {
            text: "Description:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        TextField {
            id: descriptionEdit
            text: deviceEntity.data.deviceDescription
            Layout.fillWidth: true
        }
        Button {
            id: sendConfigButton
            text: "Send"
            onClicked: {
                api.sendDeviceConfig(deviceEntity.data.deviceID);
            }
        }
// Fourth Row
        Label {
            text: "Controller Module:"
            font.bold: true
            font.underline: true
            color: "blue"
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    cursorShape = Qt.OpenHandCursor;
                }
                onExited: {
                    cursorShape = Qt.ArrowCursor;
                }

                onDoubleClicked: {
                    console.debug("CONTROLLER MODULE DOUBL-CLICKED!!!");
                }
            }
        }
        TextField {
            id: moduleEdit
            text: deviceEntity.data.controllerModuleID
            Layout.fillWidth: true
        }
        Item {
            width: 10
            height: 10
        }
// Fourth Row
        Label {
            text: "Port:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        TextField {
            id: portEdit
            text: deviceEntity.data.port
            Layout.fillWidth: true
        }
        Item {
            width: 10
            height: 10
        }
// Sixth Row
        Label {
            text: "<b>Device Class: </b>"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        ComboBox {
            id: deviceClassCombo
            model: classModel
            textRole: "text"
            Layout.minimumWidth: comboMetrics.width
            currentIndex: getClassRow(deviceEntity.data.deviceClass)
            onActivated: {
                deviceClass = classModel.get(currentIndex).classID;
            }
        }
        Item {
            width: 10
            height: 10
        }
// Seventh Row
        Label {
            text: "Serial Number (controller):"
            font.bold: true
            font.underline: true
            color: "blue"
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    cursorShape = Qt.OpenHandCursor;
                }
                onExited: {
                    cursorShape = Qt.ArrowCursor;
                }

                onDoubleClicked: {
                    console.debug("SERIAL NUMBER DOUBL-CLICKED!!!");
                }
            }
        }
        Label {
            id: serialNumber
            text: deviceEntity.data.serialNumber
            Layout.fillWidth: true
        }
        Item {
            width: 10
            height: 10
        }
// Last Row
        DeviceProperties {
            id: deviceProperties
            deviceID: deviceEntity.data.deviceID
            deviceClass: deviceEntity.data.deviceClass
            Layout.columnSpan: 3
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
