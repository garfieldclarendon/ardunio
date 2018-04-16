import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import Utils 1.0

Rectangle {
    id: mainRect
    property var routeEntity

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

    DeviceModel {
        id: deviceModel
        deviceClass: 1
    }

    RouteEntryModel {
        id: routeEntryModel
        routeID: routeEntity ? routeEntity.data.routeID : 0
    }

    ListModel {
        id: tunrouteState
        ListElement { text: "Normal"; classID: 1 }
        ListElement { text: "Diverging"; classID: 3 }
    }

    function getStateRow(turnoutState)
    {
        for(var x = 0; x < tunrouteState.count; x++)
        {
            if(tunrouteState.get(x).classID == turnoutState)
            {
                return x;
            }
        }
        return -1;
    }

    function saveData()
    {
        console.debug("saveData!!!!!!!!!!!!!!!!!!!!");
        routeEntity.setValue("routeName", nameEdit.text);
        routeEntity.setValue("routeDescription", descriptionEdit.text);
        routeEntryModel.save();
        console.debug("saveData!!!!!!!!!!!!!!!!!!!! COMPLETE");
    }

    GridLayout {
        id: gridLayout
        columns: 3
        columnSpacing: 5
        anchors.fill: parent
        anchors.margins: 2
        // First Row
        Label {
            id: routeLabel
            text: "Route ID:"
            font.bold: true
            horizontalAlignment: Qt.AlignRight
            Layout.fillWidth: true
        }
        Label {
            id: routeIDEdit
            text: routeEntity.data.routeID
            Layout.fillWidth: true
        }
        Button {
            id: saveButton
            text: "Save"
            onClicked: {
                console.debug("SAVE BUTTON CLICKED");
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
            text: routeEntity.data.routeName
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
            text: routeEntity.data.routeDescription
            Layout.fillWidth: true
        }
        Item {
            width: 10
            height: 10
        }
        //////
        Item {
            Layout.fillWidth: true
            height: 10
        }
        Label {
            Layout.fillWidth:  true
            text: "<b>Route Entries</b>"
            color: "blue"
            font.pixelSize: ui.baseFontSize + 10
            horizontalAlignment: Qt.AlignHCenter
        }
        ToolButton {
            id: addButton
            enabled: routeEntity.data.routeID > 0
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
                var entity = routeEntryModel.getEntity(-1);
                routeEntryModel.setEntity(-1, entity);
                routeView.currentIndex = routeEntryModel.getRowCount() - 1
            }
        }
        ListView {
            id: routeView
            clip: true
            model: routeEntryModel
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
            delegate: Rectangle {
                border.color: "lightgrey"
                border.width: 1
                width: parent.width
                height: deviceCombo.height + 10

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    columns: 7
                    Label {
                        text: "<b>Turnout: </b>"
                    }
                    ComboBox {
                        id: deviceCombo
                        model: deviceModel
                        textRole: "deviceName"
                        Layout.minimumWidth: comboMetrics.width
                        currentIndex: deviceModel.getDeviceRow(deviceID)
                        onActivated: {
                            deviceID = deviceModel.data(currentIndex, "deviceID");
                        }
                    }
                    Label {
                        text: "<b>State: </b>"
                        horizontalAlignment: Qt.AlignRight
                    }
                    ComboBox {
                        id: stateCombo
                        model: tunrouteState
                        textRole: "text"
                        Layout.minimumWidth: comboMetrics.width
                        currentIndex: getStateRow(turnoutState)
                        onActivated: {
                            turnoutState = model.get(currentIndex).classID;
                        }
                    }
                    ToolButton {
                        id: deleteButton
                        contentItem: Image {
                            source: "Images/delete.png"
                            sourceSize.width: 18
                            sourceSize.height: 18
                        }
                        onClicked: {
                            routeEntryModel.deleteRow(index);
                        }
                    }
                }
            }
        } //ListView
    }
}
