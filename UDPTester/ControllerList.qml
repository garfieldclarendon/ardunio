import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property alias model: listView.model
    property int controllerID: 0
    property int currentControllerClass
    property int currentSerialNumber
    property alias currentIndex: listView.currentIndex

    signal editClicked(int index);

    Component {
        id: modelDelegate
        Item {
            id: wraper
            clip: true
            property int sizeOpen: ui.applyRatio(200)
            property int sizeClosed: ui.applyRatio(110)
//            border.width: 1
//            border.color: "lightgrey"
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

            function getControllerStatus(status)
            {
                if(status === 0)
                    return "Unknown";
                else if(status === 1)
                    return "Offline";
                else if(status === 2)
                    return "Online";
                else if(status === 3)
                    return "Restarting";

                return status;
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index
                    currentControllerClass = controllerClass;
                    currentSerialNumber = serialNumber;
                    controllerID = controllerID;
                    console.debug("current index: " + index + " class: " + controllerClass + " controllerID: " + controllerID + " serial Number: " + currentSerialNumber);
                }
                onDoubleClicked: {
                    editClicked(index);
                }
            }
            GridLayout {
//                spacing: ui.margin
                columns: 3
                rows: 4
                anchors.fill: parent
                anchors.margins: ui.margin
                Text
                {
                    id: controllerNameText
                    text: controllerName
                    font.bold: true
                    font.pointSize: ui.applyFontRatio(ui.baseFontSize + 4)
                    anchors.margins: ui.margin
                    color: "blue"
                    Layout.fillWidth: true
                }
                Text {
                    text: '<b>Status:</b> ' + getControllerStatus(status)
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                    color: status === 1 ? "red" : status === 2 ? "blue" : "black"
                }
                Text {
                    text: '<b>Version:</b> ' + version
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                }
                Text
                {
                    text: '<b>ID:</b> ' + controllerID
                    anchors.margins: ui.margin
                    Layout.column: 0
                    Layout.row: 1
                }
                Text
                {
                    text: '<b>Class:</b> ' + getClassName(controllerClass)
                    anchors.margins: ui.margin
                    Layout.column: 0
                    Layout.row: 2
                }
                Text
                {
                    text: '<b>Serail #:</b> ' + serialNumber
                    anchors.margins: ui.margin
                    Layout.column: 0
                    Layout.row: 3
                }
            }
        }
    }

    ListView {
        anchors.fill: parent
        id: listView
        model: controllerModel
        spacing: ui.applyRatio(2)
        delegate: modelDelegate
        focus: true
        clip: true
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
    }
}

