import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Utils 1.0

Item {
    id: mainItem
    property alias model: listView.model
    property int controllerID: 0
    property int currentControllerClass
    property int currentSerialNumber
    property alias currentIndex: listView.currentIndex

    signal editClicked(int controllerClass, int index, var entity);
    signal addClicked(int controllerClass);
    signal deleteClicked(int index);
    signal newController(int newSerialNumber);

    ControllerModel {
        id: controllerModel
        onNewController: {
            mainItem.newController(newSerialNumber);
        }
    }

    TextMetrics {
        id: statusTextMetrics
        font.bold: true
        text: "CLASS>MULTI-CONTROLLER"
    }

    ListModel {
        id: classModel
        ListElement { text: "All"; classID: 0 }
        ListElement { text: "Multi-Controller"; classID: 7}
        ListElement { text: "Panel"; classID: 2 }
        ListElement { text: "Turnout"; classID: 1 }
        ListElement { text: "Signal"; classID: 4 }
        ListElement { text: "Semaphore"; classID: 5 }
        ListElement { text: "Block"; classID: 6 }
    }

    Component {
        id: modelDelegate
        Rectangle {
            id: wraper
            clip: true
            property int sizeOpen: ui.applyRatio(200)
            property int sizeClosed: textMetrics.boundingRect.height * 5 + 35//ui.applyRatio(130)
            border.width: ListView.isCurrentItem ? 3 : 1
            border.color: ListView.isCurrentItem ? "lightsteelblue" : "lightgrey"
            color: status == "2" ? "#E4FCFF" : "white"
            width: parent.width
            height: sizeClosed

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
/*
    ControllerStatusUnknown,
    ControllerStatusOffline,
    ControllerStatusOnline,
    ControllerStatusRestarting,
    ControllerStatusConected
  */
            function getControllerState(currentState)
            {
                if(currentState == "0")
                    return "Unknown";
                else if(currentState == "1")
                    return "Offline";
                else if(currentState == "2")
                    return "Online";
                else if(currentState == "3")
                    return "Restarting";
                else if(currentState == "4")
                    return "Connected";
                else if(currentState == "5")
                    return "Updating Firmware";
                return "Unknown";
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index
                }
                onDoubleClicked: {
                    var ent = controllerModel.getEntity(index);
                    editClicked(controllerClass, index, ent);
                }
            }
            GridLayout {
                columns: 3
                anchors.fill: parent
                anchors.margins: ui.margin
                // Line 1
                TextMetrics {
                    id: textMetrics
                    font.bold: true
                    font.pointSize: ui.applyFontRatio(ui.baseFontSize + 4)
                    text: controllerName
                }
                Text
                {
                    id: controllerNameText
                    text: textMetrics.text
                    font.bold: true
                    font.pointSize: ui.applyFontRatio(ui.baseFontSize + 4)
                    color: "blue"
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                }
                Text {
                    id: statusText
                    text: "<b>Status:</b> " + getControllerState(status)
                    verticalAlignment: Text.AlignVCenter
                    color: status == "4" ? "blue" : status == "3" ? "yellow" : "red"
                    Layout.minimumWidth: statusTextMetrics.width
                }
                // Line 2
                Text
                {
                    text: controllerDescription
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                }
                Text
                {
                    text: "<b>Class:</b> " + getClassName(controllerClass)
                }
                // Line 3
                Text
                {
                    text: '<b>Serial:</b> ' + serialNumber
                    Layout.columnSpan: 2
                }
                Button {
                    id: restartButton
                    text: "Restart"
                    onClicked: {
                        api.restartController(serialNumber);
                    }
                }

                // Line 4
                Text
                {
                    text: '<b>Controller ID:</b> ' + controllerID
                    Layout.columnSpan: 2
                }
                Text
                {
                    text: "<b>Version:</b> " + version
                }
            }
        }
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 5
        columns: 3
        Text {
            id: name
            text: qsTr("Controller Type:")
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
/*
        ClassUnknown,
        ClassTurnout,
        ClassPanel,
        ClassRoute,
        ClassSignal,
        ClassSemaphore,
        ClassBlock,
        ClassMulti,
        ClassSystem,
        ClassApp
  */
        ComboBox {
            id: controllerTypeCombo
            Layout.fillWidth: true
            model: classModel
            onCurrentIndexChanged: controllerModel.controllerClass =  classModel.get(currentIndex).classID
        }

        CRUDButtons {
            id: buttons
            enableAdd: api.apiReady
            enableDelete: listView.currentIndex >= 0 ? (api.apiReady ? true : false) : false

            onAddButtonClicked: {
                console.debug("CRUDButtons::onAddClicked ");
                addClicked(listView.model.data(listView.currentIndex, "controllerClass"));
            }
            onDeleteButtonClicked: {
                messageDialog.visible = true;
            }
            onUpdateButtonClicked:  {
                updateClicked(listView.model.data(listView.currentIndex, "controllerClass"), listView.currentIndex);
            }
        }

        ListView {
            id: listView
            model: controllerModel
            spacing: ui.applyRatio(2)
            delegate: modelDelegate
            focus: true
            clip: true
            Layout.columnSpan: 3
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
    MessageDialog {
        id: messageDialog
        title: "Delete Controller"
        text: "Are you sure you want to delete the currently selected controller?"
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            controllerModel.deleteRow(listView.currentIndex);
            controllerModel.save();
            visible = false;
        }
        onNo: {
            visible = false;
        }

        Component.onCompleted: visible = false
    }
}

