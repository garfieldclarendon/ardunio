import QtQuick 2.9
import QtQuick.Layouts 1.3
import Utils 1.0

Rectangle {
    property alias model: deviceModel
    property alias currentIndex: deviceList.currentIndex

    border.width: 1
    border.color: "black"

    signal itemDoubleClicked(int deviceID, int deviceClass)

    function getDeviceState(currentState, deviceClass)
    {
        if(deviceClass === "1")
            return getTurnoutState(currentState);
        else if(deviceClass === "6")
            return getBlockState(currentState);

        return currentState;
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
        console.debug("getTurnoutState: " + currentState);
        if(currentState === 0)
            return "Unknown";
        else if(currentState === 1)
            return "Normal";
        else if(currentState === 2)
            return "To Diverging";
        else if(currentState === 3)
            return "Diverging";
        else if(currentState === 4)
            return "To Normal";

        return currentState;
    }

    function getClassName(classID)
    {
        if(classID === "0")
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

    DeviceModel {
        id: deviceModel
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        Item {
            id: header
            Layout.fillWidth: true
            height: idText.height + 10

            GridLayout {
                anchors.fill: parent
                anchors.margins: 5

                Text {
                    id: idText
                    color: "blue"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    Layout.maximumWidth: parent.width * .05
                    text: qsTr("ID")
                }
                Text {
                    id: nameText
                    font.bold: true
                    color: "blue"
                    horizontalAlignment: Text.AlignHCenter
                    Layout.minimumWidth: parent.width * .15
                    text: qsTr("Name")
                }
                Text {
                    id: classText
                    font.bold: true
                    color: "blue"
                    Layout.minimumWidth: parent.width * .05
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Class")
                }
                Text {
                    id: stateText
                    font.bold: true
                    color: "blue"
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("State")
                }
            }
        }

        ListView {
            id: deviceList
            model: deviceModel
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
            focus: true

            delegate: Item {
                id: listDelegate
                height: id.height + 15
                width: parent.width
                Rectangle {
                    anchors.fill: parent
                    border.width: 1
                    anchors.margins: 2
                    border.color: "lightgrey"
                    GridLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        Text {
                            id: id
                            text: deviceID
                            Layout.minimumWidth: idText.width
                            Layout.maximumWidth: idText.width
                        }
                        Text {
                            id: name
                            text: deviceName
                            Layout.minimumWidth: nameText.width
                            Layout.maximumWidth: nameText.width
                        }
                        Text {
                            id: deviceClassText
                            text: getClassName(deviceClass)
                            Layout.minimumWidth: classText.width
                            Layout.maximumWidth: classText.width
                        }
                        Text {
                            id: deviceStateText
                            text: deviceState ? getDeviceState(deviceState, deviceClass) : "N/A"
                            Layout.minimumWidth: stateText.width
                            Layout.maximumWidth: stateText.width
                        }
                    }
                }
                MouseArea {
                          anchors.fill: parent
                          onClicked: {
                              deviceList.currentIndex = index
                          }

                          onDoubleClicked: {
                              deviceList.currentIndex = index
                              itemDoubleClicked(deviceID, deviceClass)
                      }
                }
            }
        }
    }
}
