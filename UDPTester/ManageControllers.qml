import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import Utils 1.0


Item {
    id: mainItem
    anchors.margins: ui.margin

    ControllerModel {
        id: controllerModel
        onNewController: {
            newController.serialNumber = serialNumber;
            newController.classID = controllerClass;
            fadeIn.start();
        }
    }

    function showEdit(row)
    {
        stackView.push({item:"qrc:/EditController.qml", properties:{editModel:controllerModel,index:row}})
        stackView.currentItem.editFinished.connect(editFinished);
        if(newController.height > 0)
            fadeOut.start();
    }

    function editFinished(save)
    {
        stackView.pop();
        if(save)
            controllerModel.save();
    }

    function controllerSelected(controllerID)
    {
        console.debug("CONTROLLER SELECTED!  : " + controllerID + " FOR SERIAL NUMBER " + newController.serialNumber);
        stackView.pop();

        controllerModel.setNewSerialNumber(controllerID, newController.serialNumber);
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: ui.margin

        RowLayout {
            id: buttonLayout
            Layout.fillWidth: true

            Item {
                Layout.fillWidth: true
            }

            Button {
                id: addButton
                text: "Add"

                onClicked: {
                    console.debug("Add Clicked");
                    var newRow = controllerModel.addNew();
                    showEdit(newRow);
                }
            }

            Button {
                id: resetAllButton
                text: "Reset (All)"
                onClicked: {
                    console.debug("Reset (All) Button Clicked!!");
                    broadcaster.sendResetCommand(0);
                }
            }

            Button {
                id: sendConfigAllButton
                text: "Send Config (All)"
                onClicked: {
                    console.debug("Send Config (All) Button Clicked!!");
                    broadcaster.sendConfigData(0);
                }
            }

            Button {
                id: updateFirmwareAllButton
                text: "Update Firmare (All)"
                onClicked: {
                    console.debug("Update Firmware (All) Button Clicked!!");
                    broadcaster.sendDownloadFirmware(0);
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            anchors.margins: ui.margin

            ColumnLayout {
                Layout.preferredWidth: mainItem.width * .55
                anchors.margins: ui.margin

                NewController {
                    id: newController
                    Layout.fillWidth: true
                    height: ui.applyRatio(100)
                    visible: false
                    opacity: 0
                    onEditFinished: {
                        fadeOut.start();
                    }
                    onOpacityChanged: {
                        if(opacity === 0.0)
                            visible = false;
                        else
                            visible = true;
                    }
                    onAddController: {
                        var newRow = controllerModel.addNew();
                        controllerModel.setData(newRow, "serialNumber", newController.serialNumber);
                        controllerModel.setData(newRow, "controllerClass", newController.classID);
                        showEdit(newRow);
                    }
                    onReplaceController: {
                        if(newController.height > 0)
                            fadeOut.start();
                        stackView.push({item:"qrc:/LookupController.qml" })
                        stackView.currentItem.controllerSelected.connect(controllerSelected);
                    }
                }

                StackView {
                    id: stackView
                    clip:true
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Component {
                        id: list
                        ControllerList {
                            id: controllerList
                            model: controllerModel
                            onEditClicked: {
                                showEdit(index);
                             }
                            onControllerIDChanged: {
                                console.debug();
                                if(currentControllerClass === 1)
                                {
                                    detailLoader.setSource("TurnoutController.qml",
                                                                 { "currentControllerID": controllerID });
                                }
                                else if(currentControllerClass === 2)
                                {
                                    detailLoader.setSource("PanelController.qml",
                                                                 { "currentControllerID": controllerID });
                                }
                                else
                                {
                                    detailLoader.source = "";
                                }
                            }
                        }
                    }

                    Component.onCompleted: {
                        stackView.push(list);
                    }
                }
            }

            Loader {
                id: detailLoader
                Layout.preferredWidth: mainItem.width * .35 - 10
                anchors.margins: ui.margin
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
        ParallelAnimation {
            id: fadeIn
            running: false
            NumberAnimation { target: newController; easing.type: Easing.OutSine; property: "opacity"; to: 1; duration: 500; }
        }
        ParallelAnimation {
            id: fadeOut
            running: false
            NumberAnimation { target: newController; easing.type: Easing.OutSine; property: "opacity"; to: 0; duration: 500; }
        }
    }
}

