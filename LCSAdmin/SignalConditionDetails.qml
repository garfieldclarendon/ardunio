import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import Utils 1.0

Rectangle {
    id: main

    signal closeButtonClicked()

    property var conditionModel

    height: 100
    width: parent.width

    DeviceModel {
        id: deviceModel
    }

    TextMetrics {
        id: fontMetrics
        font.bold: true
        font.pointSize: ui.baseFontSize + 10
        text: "Signal Details"
    }

    TextMetrics {
        id: comboMetrics
        text: "NOT EQUALS TO ME"
    }

    ListModel {
        id: operandModel
        ListElement { text: "Equals"; mode: 0 }
        ListElement { text: "Not Equals"; mode: 1 }
        ListElement { text: "Greater Than"; mode: 2 }
        ListElement { text: "Less Than"; mode: 3 }
    }

    ListModel {
        id: turnoutStateModel
        ListElement { text: "Normal"; mode: 1 }
        ListElement { text: "Diverging"; mode: 3 }
    }
    ListModel {
        id: blockStateModel
        ListElement { text: "Unoccupied"; mode: 1 }
        ListElement { text: "Occupied"; mode: 2 }
    }

    function getModelIndex(model, value)
    {
        for(var x = 0; x < model.count; x++)
        {
            if(model.get(x).mode == value)
            {
                return x;
            }
        }
        return -1;
    }

    GridLayout {
        anchors.fill: parent
        columns: 5
        Item {
            height: addButton.height
            Layout.fillWidth: true
        }
        Text {
            text: "<b>Signal Conditions</b>"
            color: "blue"
//            Layout.minimumWidth: fontMetrics.width * 2
            font.pixelSize: ui.baseFontSize + 10
            horizontalAlignment: Qt.AlignHCenter
        }
        ToolButton {
            id: addButton
            contentItem: Image {
                source: "Images/add.png"
                sourceSize.width: 18
                sourceSize.height: 18
            }
            onClicked: {
                var entity = conditionModel.getEntity(-1);
                conditionModel.setEntity(-1, entity);
                conditionView.currentIndex = conditionView.rowCount - 1
            }
        }

        Button {
            id: saveButton
            text: "Save"
            onClicked: {
                conditionModel.save();
                closeButtonClicked();
            }
        }
        Button {
            id: cancelButton
            text: "Cancel"
            onClicked: {
                closeButtonClicked();
            }
        }

        ListView {
            id: conditionView
            clip: true
            model: conditionModel
            Layout.columnSpan: 5
            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: Rectangle {
                border.color: "lightgrey"
                border.width: 1
                width: parent.width
                height: deviceCombo.height + 10

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: {
                        aspectDoubleClicked(signalAspectID);
                    }
                }

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    columns: 7
                    Item {
                        Layout.fillWidth: true
                        height: 10
                    }

                    Label {
                        text: "<b>Device: </b>"
                        horizontalAlignment: Qt.AlignRight
                    }
                    ComboBox {
                        id: deviceCombo
                        model: deviceModel
                        textRole: "deviceName"
                        currentIndex: deviceModel.getDeviceRow(deviceID)
                        Layout.minimumWidth: comboMetrics.width
                        onActivated: {
                            deviceID = deviceModel.data(currentIndex, "deviceID");
                        }
                        onCurrentIndexChanged: {
                            var c = deviceModel.data(currentIndex, "deviceClass");
                            console.debug("deviceCombo:onCurrentIndexChanged: " + c)
                            if(c == 1)
                            {
                                stateCombo.model = turnoutStateModel;
                            }
                            else if(c == 6)
                            {
                                stateCombo.model = blockStateModel;
                            }
                            else
                            {
                                var model;
                                stateCombo.model = model;
                            }
                        }
                     }
                    ComboBox {
                        id: operandCombo
                        model: operandModel
                        textRole: "text"
                        currentIndex: conditionOperand
                        Layout.minimumWidth: comboMetrics.width
                        onActivated: {
                            conditionOperand = operandCombo.currentIndex;
                        }
                     }
//                    Label {
//                        text: "<b>Device State: </b>"
//                    }
                    ComboBox {
                        id: stateCombo
                        textRole: "text"
                        currentIndex: model ? getModelIndex(stateCombo.model, deviceState) : -1
                        Layout.minimumWidth: comboMetrics.width
                        onActivated: {
                            deviceState = stateCombo.model.get(stateCombo.currentIndex).mode;
                        }
                     }
                    Item {
                        Layout.fillWidth: true
                        height: 10
                    }
                    ToolButton {
                        id: deleteButton
                        contentItem: Image {
                            source: "Images/delete.png"
                            sourceSize.width: 18
                            sourceSize.height: 18
                        }
                        onClicked: {
                            conditionModel.deleteRow(index);
                        }
                    } // ToolButton
                } // delegate GridLayout
            } // delegate Rectangle
        } // ListView
    } // GridLayout
} // Main Rectangle
