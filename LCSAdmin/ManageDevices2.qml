import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property alias model: listView.model
    property int controllerID: 0
    property int currentdeviceClass
    property int currentSerialNumber
    property alias currentIndex: listView.currentRow

    signal editClicked(int deviceClass, int index, var entity);
    signal addClicked(int deviceClass);
    signal deleteClicked(int index);

    DeviceModel {
        id: deviceModel
        deviceClass: 1
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

    function getDeviceState(deviceClass, currentState)
    {
        if(deviceClass === "1")
            return getTurnoutState(currentState);
        else if(deviceClass === "1")
            return getTurnoutState(currentState);

        return "N/A";
    }

    function getBlockState(currentState)
    {
        if(currentState === 0)
            return "Unknown";
        else if(currentState === 1)
            return "Unoccupied";
        else if(currentState === 2)
            return "Occupied";

        return currentState;
    }

    function getTurnoutState(currentState)
    {
        if(currentState === 0)
            return "Unknown";
        else if(currentState === 1)
            return "Normal";
        else if(currentState === 2)
            return "ToDiverging";
        else if(currentState === 3)
            return "Diverging";
        else if(currentState === 4)
            return "ToNormal";

        return currentState;
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 5
        columns: 3
        Text {
            id: name
            text: qsTr("Device Type:")
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
            id: deviceTypeCombo
            Layout.fillWidth: true
            model: ListModel {
                id: classModel
                ListElement { text: "All"; classID: 0 }
                ListElement { text: "Turnout"; classID: 1 }
                ListElement { text: "Signal"; classID: 4 }
                ListElement { text: "Semaphore"; classID: 5 }
                ListElement { text: "Block"; classID: 6 }
            }
            onCurrentIndexChanged: deviceModel.deviceClass =  classModel.get(currentIndex).classID
        }

        CRUDButtons {
            id: buttons
            enableAdd: api.apiReady
            enableDelete: listView.currentIndex >= 0 ? (api.apiReady ? true : false) : false

            onAddButtonClicked: {
                console.debug("CRUDButtons::onAddClicked ");
                addClicked(listView.model.data(listView.currentIndex, "deviceClass"));
            }
            onDeleteButtonClicked: {
                deleteClicked(listView.currentIndex);
            }
            onUpdateButtonClicked:  {
                updateClicked(listView.model.data(listView.currentIndex, "deviceClass"), listView.currentIndex);
            }
        }

        TableView {
            id: listView
            model: deviceModel
            focus: true
            clip: true
            Layout.columnSpan: 3
            Layout.fillHeight: true
            Layout.fillWidth: true
            onClicked: {
                listView.currentRow = row
            }
            onDoubleClicked: {
                var ent = deviceModel.getEntity(row);
                editClicked(deviceModel.data(row, "deviceClass"), row, ent);
            }
            TableViewColumn {
                role: "deviceID"
                title: "ID"
                width: 50
            }
            TableViewColumn {
                role: "deviceName"
                title: "Name"
                width: 150
            }
            TableViewColumn {
                role: "deviceDescription"
                title: "Description"
                width: 150
            }
            TableViewColumn {
                role: "deviceClass"
                title: "Class"
                width: 70
                delegate:
                    Text {
                    font.pointSize: ui.baseFontSize
                        text: getClassName(styleData.value)
                    }
            }
            TableViewColumn {
                role: "port"
                title: "Port"
                width: 75
            }
            TableViewColumn {
                role: "serialNumber"
                title: "Serial #"
                width: 95
            }
            TableViewColumn {
                role: "controllerID"
                title: "Con. ID"
                width: 95
            }
            TableViewColumn {
                role: "deviceState"
                title: "State"
                width: 75
                delegate:
                    Text {
                        font.bold: true
                        text: getDeviceState(deviceModel.data(styleData.row, "deviceClass"), styleData.value)
                    }
            }
            TableViewColumn {
                role: "deviceClass"
                title: ""
                width: 50
                delegate:
                    Button {

                         text: "Activate"
                         enabled: styleData.value === "1" ? true : false
                         onClicked: {
                             api.activateTurnout(deviceModel.data(styleData.row, "deviceID"), 0);
                         }
                    }
            }
        }
    }
}

