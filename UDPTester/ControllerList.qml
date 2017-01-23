import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property alias model: listView.model
    property int controllerID: 0
    property int currentControllerClass
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
                else if(classID === 1)
                    return "Turnout";
                else if(classID === 2)
                    return "Panel";
                else if(classID === 3)
                    return "Route";
                else if(classID === 4)
                    return "Signal";
                else if(classID === 5)
                    return "Semaphore";
                else if(classID === 6)
                    return "Block";
                else if(classID === 7)
                    return "System";
                else if(classID === 8)
                    return "Application";

                return "";
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    listView.currentIndex = index
                    currentControllerClass = controllerClass;
                    controllerID = id;
                }
                onDoubleClicked: {
                    editClicked(index);
                }
            }
            ColumnLayout {
                spacing: ui.margin
                //width: parent.width
                anchors.fill: parent
                anchors.margins: ui.margin
                RowLayout {
                    spacing: ui.margin
                    Text
                    {
                        id: controllerNameText
                        text: controllerName
                        font.bold: true
                        font.pointSize: ui.applyFontRatio(ui.baseFontSize + 4)
                        anchors.margins: ui.margin
                        Layout.minimumWidth:  ui.applyRatio(300)
                        color: "blue"
                    }
                    Text {
                        text: '<b>Status:</b> ' + currentStatus
                        verticalAlignment: Text.AlignVCenter
                        height: parent.height
                        color: currentStatus === "Offline" ? "red" : currentStatus === "?" ? "black" : "blue"
                    }

                    Text {
                        text: '<b>Version:</b> ' + version
                        verticalAlignment: Text.AlignVCenter
                        height: parent.height
                    }
                }
                Text
                {
                    text: '<b>ID:</b> ' + id
                    anchors.margins: ui.margin
                }
                Text
                {
                    text: '<b>Class:</b> ' + getClassName(controllerClass)
                    anchors.margins: ui.margin
                }
                Text
                {
                    text: '<b>Serail #:</b> ' + serialNumber
                    anchors.margins: ui.margin
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
        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
    }
}

