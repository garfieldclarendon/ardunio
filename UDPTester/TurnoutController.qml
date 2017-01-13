import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    id: outerRect
    property int currentControllerID: 0

    TurnoutModel {
        id: turnoutModel
        controllerID: currentControllerID
    }

//    radius: 20
//    border.color: "grey"

    Component {
        id: turnoutDelegate
        Rectangle {
            id: wraper
            clip: true
            width: outerRect.width - ui.applyRatio(35)
            height: ui.applyRatio(60)
            border.color: "grey"
            border.width: 1
            color: id > 0 ? "white" : "lightgrey"
            function getStateText(state)
            {
                if(state === "0" || state === "?")
                    return "Unknown";
                else if(state === "1")
                    return "Normal";
                else if(state === "2")
                    return "ToDiverging";
                else if(state === "3")
                    return "Diverging";
                else if(state === "4")
                    return "ToNormal";
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.debug("Mouse Clicked!")
                    listView.currentIndex = index
                }
            }
            ColumnLayout {
                spacing: ui.margin
                anchors.fill: parent
                anchors.margins: ui.margin
                RowLayout {
                    Rectangle {
                        color: "red"
                        width: 15
                        height: 25
                    }
                    Text {
                        text: "hello???"
                    }

                    Text
                    {
                        id: controllerNameText
                        text: "Testing" //id > 0 ? itemName : ""
                        font.bold: true
//                        font.pointSize: ui.applyFontRatio(ui.baseFontSize + 4)
//                        anchors.margins: ui.margin
//                        Layout.minimumWidth:  ui.applyRatio(100)
                        color: "blue"
                    }
                    Rectangle {
                        color: "green"
                        width: 15
                        height: 25
                    }
                    ColumnLayout {
                        Layout.minimumWidth: ui.applyRatio(200)
                        Text {
                            text: id > 0 ? '<b>Description:</b> ' + itemDescription : ""
                            verticalAlignment: Text.AlignVCenter
                            height: 50 //parent.height
                         }
                        Text {
                            text: id > 0 ? '<b>ID:</b> ' + id : ""
                            verticalAlignment: Text.AlignVCenter
                            height: 50 //parent.height
                         }
                        Text {
                            text: id > 0 ? '<b>State:</b> ' + getStateText(currentState) : ""
                            verticalAlignment: Text.AlignVCenter
                            height: parent.height
                         }
                    }
                    ColumnLayout {
                        Button {
                            id: activate
                            text: "Activate"
                            enabled: currentState != "?" && id > 0
                            onClicked: {
                                console.debug("current state: " + currentState);
                                broadcaster.sendMessage(102, currentControllerID, id, 1, 0, id, 0, currentState === "1" ? 2 : 1, 0);
                            }
                        }
                        Button {
                            id: actionButton
                            text: id > 0 ? "Remove" : "Add"
                            onClicked: {
                                console.debug("Action button clicked");
                            }
                        }
                    }
                }
            }
        }
    }

    ListView {
        anchors.fill: parent
        anchors.margins: ui.applyRatio(15)
        id: listView
        model: turnoutModel
        delegate: turnoutDelegate
        spacing: ui.applyRatio(2)
        focus: true
    }
}

