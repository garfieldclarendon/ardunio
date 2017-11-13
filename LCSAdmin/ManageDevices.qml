import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property alias model: listView.model
    property int controllerID: 0
    property int currentdeviceClass
    property int currentSerialNumber
    property alias currentIndex: listView.currentIndex

    signal editClicked(int deviceClass, int index, var entity);
    signal addClicked(int deviceClass);
    signal deleteClicked(int index);

    DeviceModel {
        id: deviceModel
        deviceClass: 1
    }

    Component {
        id: modelDelegate
        Rectangle {
            id: wraper
            clip: true
            property int sizeOpen: ui.applyRatio(200)
            property int sizeClosed: textMetrics.boundingRect.height * 7 + 10//ui.applyRatio(130)
            border.width: ListView.isCurrentItem ? 3 : 1
            border.color: ListView.isCurrentItem ? "lightsteelblue" : "lightgrey"
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

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index
                }
                onDoubleClicked: {
                    var ent = deviceModel.getEntity(index);
                    editClicked(deviceClass, index, ent);
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
                    text: deviceName
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
                    text: '<b>Status:</b> ' + getDeviceState(deviceClass, deviceState)
                    verticalAlignment: Text.AlignVCenter
//                    color: getCurrentState(deviceState)
                }
                // Line 2
                Text
                {
                    text: deviceDescription
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                }
                Text
                {
                    text: '<b>Class:</b> ' + getClassName(deviceClass)
                }
                // Line 3
                Text
                {
                    text: '<b>Serial:</b> ' + serialNumber
                    Layout.columnSpan: 3
                }
                // Line 4
                Text
                {
                    text: '<b>Controller ID:</b> ' + controllerID
                    Layout.columnSpan: 3
                }
                // Line 5
                Text
                {
                    text: '<b>Module Address:</b> ' + address
                    Layout.columnSpan: 3
                }
                Text
                {
                    text: '<b>Port:</b> ' + port
                    Layout.columnSpan: 3
                }
                // Line 6
                Text
                {
                    text: '<b>Device ID:</b> ' + deviceID
                    Layout.columnSpan: 3
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

        ListView {
            id: listView
            model: deviceModel
            spacing: ui.applyRatio(2)
            delegate: modelDelegate
            focus: true
            clip: true
            Layout.columnSpan: 3
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}

