import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import Utils 1.0

Rectangle {
    id: mainRect
    property var controllerEntity

    signal saveClicked();
    signal cancelClicked();

    implicitHeight: saveButton.height * 7
    width: 400

    //    border.color: "red"
    //    border.width: 1
    clip: true

    TextMetrics {
        id: comboMetrics
        text: "CONTROLLER NAME"
    }

    ControllerModuleModel {
        id: moduleModel
        controllerID: controllerEntity.data ? controllerEntity.data.controllerID : 0
    }

    ListModel {
        id: classModel
        ListElement { text: "Unknown"; classID: 0 }
        ListElement { text: "Turnout"; classID: 1 }
        ListElement { text: "Signal"; classID: 4 }
        ListElement { text: "Semaphore"; classID: 5 }
        ListElement { text: "Block"; classID: 6 }
        ListElement { text: "Panel"; classID: 2 }
        ListElement { text: "Multi-Controller"; classID: 7}
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
        console.debug("saveData!!!!!!!!!!!!!!!!!!!!");
        controllerEntity.setValue("controllerName", nameEdit.text);
        controllerEntity.setValue("controllerDescription", descriptionEdit.text);
        controllerEntity.setValue("serialNumber", serialNumberEdit.text);
        controllerEntity.setValue("controllerClass", classModel.get(classCombo.currentIndex).classID);
        moduleModel.save();
        console.debug("saveData!!!!!!!!!!!!!!!!!!!! COMPLETE");
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
            id: controllerLabel
            text: "Controller ID:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        Label {
            id: controllerIDEdit
            text: controllerEntity.data.controllerID
            Layout.fillWidth: true
        }
        Button {
            id: saveButton
            text: "Save"
            onClicked: {
                saveData();
                saveClicked();
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
            text: controllerEntity.data.controllerName
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
            text: controllerEntity.data.controllerDescription
            Layout.fillWidth: true
        }
        Button {
            id: sendConfigButton
            text: "Send Config"
            onClicked: {
                api.sendControllerConfig(controllerEntity.data.serialNumber);
            }
        }
        // Fourth Row
        Label {
            text: "Controller Class:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        ComboBox {
            id: classCombo
            model: classModel
            textRole: "text"
            Layout.fillWidth: true
            currentIndex: getClassRow(controllerEntity.data.controllerClass)
            onActivated: {
                controllerClass = classModel.get(currentIndex).classID;
            }
        }
        Item {
            Layout.fillWidth: true
            height: 10
        }
        // Fifth Row
        Label {
            text: "Serial Number:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        TextField {
            id: serialNumberEdit
            text: controllerEntity.data.serialNumber
            Layout.fillWidth: true
        }
        Item {
            width: 10
            height: 10
        }
        Item {
            width: addButton.width
            height: addButton.height
        }
        Label {
            Layout.fillWidth:  true
            text: "<b>Modules</b>"
            color: "blue"
            font.pixelSize: ui.baseFontSize + 10
            horizontalAlignment: Qt.AlignHCenter
        }
        ToolButton {
            id: addButton
            enabled: controllerEntity.data.controllerClass === "7" ? moduleModel.rowCount < 9 ? true : false : moduleModel.rowCount === 1 ? false : true
            contentItem: Image {
                source: "Images/add.png"
                sourceSize.width: 18
                sourceSize.height: 18
                layer.enabled: true
                layer.effect: Desaturate {
                    desaturation: parent.enabled ? 0 : 1
                    Behavior on desaturation { NumberAnimation { easing.type: Easing.InOutQuad } }
                }
            }
            onClicked: {
                var entity = moduleModel.getEntity(-1);
                moduleModel.setEntity(-1, entity);
                moduleView.currentIndex = moduleModel.getRowCount() - 1
            }
        }
        ListView {
            id: moduleView
            clip: true
            model: moduleModel
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
            delegate: Rectangle {
                border.color: "lightgrey"
                border.width: 1
                width: parent.width
                height: moduleNameEdit.height + 10

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    columns: 9
                    Label {
                        text:"<b>Module Name:</b>"
                        horizontalAlignment: Qt.AlignRight
                    }
                    TextField {
                        id: moduleNameEdit
                        text: moduleName
                    }
                    Label {
                        text: "<b>Index (address): </b>"
                        horizontalAlignment: Qt.AlignRight
                    }
                    TextField {
                        Layout.maximumWidth: 35
                        id: modueIndexEdit
                        text: address
                    }
                    Label {
                        text: "Module Class:"
                        font.bold: true
                        horizontalAlignment: Qt.AlignRight
                    }
                    ComboBox {
                        id: moduleClassCombo
                        model: classModel
                        textRole: "text"
                        Layout.fillWidth: true
                        currentIndex: getClassRow(moduleClass)
                        onActivated: {
                            moduleClass = classModel.get(currentIndex).classID;
                        }
                    }
                    Label {
                        text: "<b>Disable: </b>"
                        horizontalAlignment: Qt.AlignRight
                    }
                    CheckBox {
                        Layout.maximumWidth: 35
                        id: disableCheckBox
                        checked: disable == 1
                    }
                    ToolButton {
                        id: deleteButton
                        contentItem: Image {
                            source: "Images/delete.png"
                            sourceSize.width: 18
                            sourceSize.height: 18
                        }
                        onClicked: {
                            moduleModel.deleteRow(index);
                        }
                    }
                }
            }
        } //ListView
    }
}
