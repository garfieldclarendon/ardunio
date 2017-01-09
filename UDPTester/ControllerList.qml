import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property alias model: listView.model
    property int controllerID: 0
    property int currentControllerClass

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
            height: getSize()

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
                    return "Signal/Block";
                else if(classID === 5)
                    return "System";
                else if(classID === 6)
                    return "Application";

                return "";
            }

            function getSize()
            {
                if(!ListView.isCurrentItem)
                {
                    doClose.start();
                    fontRestore.start();
                    return sizeClosed;
                }

                doOpen.start();
                fontExpand.start();
                return sizeOpen;
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
                        font.pointSize: ui.applyFontRatio(ui.baseFontSize)
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
                Rectangle {
                    width: ui.applyRatio(490)
                    height: ui.applyRatio(40)
                    visible: wraper.ListView.isCurrentItem
                    color: "lightgrey"
                    border.color: "grey"
                    border.width: 1
                    radius: ui.applyRatio(10)
                    RowLayout {
                        anchors.centerIn: parent
                        spacing: ui.margin
                        Button {
                            id: editButton
                            text: "Edit"
                            onClicked: {
                                console.debug("Edit Button Clicked!!");
                                editClicked(index);
                             }
                        }
                        Button {
                            id: resetButton
                            text: "Reset"
                            onClicked: {
                                console.debug("Reset Button Clicked!!");
                                broadcaster.sendResetCommand(id);
                            }
                        }
                        Button {
                            id: sendConfig
                            text: "Send Config"
                            onClicked: {
                                console.debug("Send Config Button Clicked!!");
                                broadcaster.sendConfigData(id);
                            }
                        }
                        Button {
                            id: updateFirmware
                            text: "Update Firmware"
                            onClicked: {
                                console.debug("Update Firmware Button Clicked!!");
                                broadcaster.sendDownloadFirmware(id);
                            }
                        }
                    }
                }
            }
            ParallelAnimation {
                id: doOpen
                running: false
                NumberAnimation { target: wraper; easing.type: Easing.OutSine; property: "height"; to: sizeOpen; duration: 500; }
            }
            ParallelAnimation {
                id: doClose
                running: false
                NumberAnimation { target: wraper; easing.type: Easing.OutSine; property: "height"; to: sizeClosed; duration: 500; }
            }
            ParallelAnimation {
                id: fontExpand
                running: false
                NumberAnimation { target: controllerNameText; easing.type: Easing.OutSine; property: "font.pointSize"; to: ui.applyFontRatio(ui.baseFontSize + 4); duration: 500; }
            }
            ParallelAnimation {
                id: fontRestore
                running: false
                NumberAnimation { target: controllerNameText; easing.type: Easing.OutSine; property: "font.pointSize"; to: ui.applyFontRatio(ui.baseFontSize); duration: 500; }
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
    }
}

