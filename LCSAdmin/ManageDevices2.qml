import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3
import Utils 1.0
import Track 1.0

Item {
    property alias model: listView.model
    property int controllerID: 0
    property int currentdeviceClass
    property int currentSerialNumber
    property alias currentIndex: listView.currentRow

    signal editClicked(int deviceClass, int index, var entity);
    signal addClicked(int deviceClass);
    signal deleteClicked(int index);

    TextMetrics {
        id: textMetrics
//        font.family: "Courier New"
//        font.pixelSize: 25
//        font.pointSize: ui.baseFontSize
        font.bold: false
        text: "CLASS>MULTI-CONTROLLER"
    }

    DeviceModel {
        id: deviceModel
        onClassChanged: {
            if(deviceClass == 0)
                deviceTypeCombo.currentIndex = 0
        }
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
    function getClassName(classID)
    {
        if(classID === 0)
            return "Unknown";
        else if(classID === "1")
            return "Turnout";
        else if(classID === "2")
            return "Panel Input";
        else if(classID === "3")
            return "Panel Output";
        else if(classID === "4")
            return "Signal";
        else if(classID === "5")
            return "Semaphore";
        else if(classID === "6")
            return "Block";

        return classID;
    }

    function getDeviceState(deviceClass, currentState)
    {
        if(deviceClass === "1")
            return getTurnoutState(currentState);
        else if(deviceClass === "6")
            return getBlockState(currentState);

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
//        anchors.margins: 5
        columns: 4
        TrackSingle {
            isActive: false
            model: deviceModel
            deviceID: 1
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
        }
        TrackTurnout {
            isActive: true
            model: deviceModel
            deviceID: 4
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
        }
        TrackSingle {
            isActive: true
            blockStatus: 2
            model: deviceModel
            deviceID: 5
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
        }
        TrackSingle {
            isActive: true
            blockStatus: 3
            model: deviceModel
            deviceID: 6
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
        }

        TextField {
            id: searchText
            placeholderText: "Enter Search Text"
            text: deviceModel.filterText
            inputMethodHints: Qt.ImhNoPredictiveText
            Layout.fillWidth: true
            onTextChanged: {
                deviceModel.filterText = searchText.text;
            }
        }

        Text {
            id: name
            horizontalAlignment: Text.AlignRight
            text: qsTr("Filter By:")
            font.bold: true
            Layout.fillWidth: true
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
        ComboBox {
            id: deviceTypeCombo
            Layout.fillWidth: true
            model: ListModel {
                id: classModel
                ListElement { text: "All"; classID: 0 }
                ListElement { text: "Turnout"; classID: 1 }
                ListElement { text: "Panel Input"; classID: 2 }
                ListElement { text: "Panel Output"; classID: 3 }
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
            alternatingRowColors: true
            Layout.columnSpan: 4
            Layout.fillHeight: true
            Layout.fillWidth: true
            onClicked: {
                listView.currentRow = row
            }
            onDoubleClicked: {
                var ent = deviceModel.getEntity(row);
                editClicked(deviceModel.data(row, "deviceClass"), row, ent);
            }
            rowDelegate: Rectangle{
                width: parent.width
                height: textMetrics.height + 10
                color: styleData.selected ? "#448" : (styleData.alternate? "#eee" : "#fff")
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
                        verticalAlignment: Text.AlignVCenter
                        elide: styleData.elideMode
                        font: textMetrics.font
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
                        verticalAlignment: Text.AlignVCenter
                        elide: styleData.elideMode
                        font: textMetrics.font
                        text: getDeviceState(deviceModel.data(styleData.row, "deviceClass"), styleData.value)
                    }
            }
            TableViewColumn {
                role: "deviceClass"
                title: ""
                width: 120
                delegate:
                    Button {
                         text: "Activate"
                         style: ButtonStyle {
                             label: Text {
                                 font: textMetrics.font
                                 verticalAlignment: Text.AlignVCenter
                                 horizontalAlignment: Text.AlignHCenter
                                 text: "Activate"
                             }
                         }

                         enabled: styleData.value === "1" ? true : false
                         onClicked: {
                             api.activateTurnout(deviceModel.data(styleData.row, "deviceID"), 0);
                         }
                    }
            }
            itemDelegate: Text {
                property variant value: styleData.value
                verticalAlignment: Text.AlignVCenter
                height: textMetrics.height
                elide: styleData.elideMode
                text: styleData.value !== undefined ? styleData.value : ""
                renderType: Text.NativeRendering
                font: textMetrics.font
            }
        }
    }
}

