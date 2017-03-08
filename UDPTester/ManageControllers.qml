import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Rectangle {
    id: mainItem
    color: systemPalette.window
    anchors.margins: ui.margin

    SystemPalette {
        id: systemPalette
    }

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
                Layout.maximumWidth: Math.min(mainItem.width * .40, 500)
                Layout.minimumWidth: 500
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
                    property bool allowAddNew: false
                    onAllowAddNewChanged: {
                        controllerActions.enableAddModule = allowAddNew;
                        console.debug("ControllerList::onAllowAddNewChanged: " + allowAddNew);
                    }
                    Component {
                        id: list

                        ControllerList {
                            id: controllerList

                            model: controllerModel
                            onEditClicked: {
                                showEdit(index);
                             }

                            onControllerIDChanged: {
                                console.debug("onControllerIDChanged - SerialNumber: " + currentSerialNumber);
                                if(currentControllerClass === 1)
                                {
                                    detailLoader.setSource("MultiModule.qml",
                                                           { "currentControllerID": controllerID, "currentControllerClass": currentControllerClass, "serialNumber": currentSerialNumber});
                                }
                                else if(currentControllerClass === 2)
                                {
                                    detailLoader.setSource("PanelController.qml",
                                                                 { "currentControllerID": controllerID, "serialNumber": currentSerialNumber });
                                }
                                else if(currentControllerClass === 7)
                                {
                                    detailLoader.setSource("MultiModule.qml",
                                                           { "currentControllerID": controllerID, "currentControllerClass": currentControllerClass, "serialNumber": currentSerialNumber });
                                }
                                else
                                {
                                    detailLoader.setSource("MultiModule.qml",
                                                                 { "currentControllerID": controllerID, "currentControllerClass": currentControllerClass, "serialNumber": currentSerialNumber });
                                }
                            }
                        }
                    }

                    Component.onCompleted: {
                        stackView.push(list);
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                ControllerActions {
                    id: controllerActions
                    enableAddModule: true
                    enableAddDevice: true
                    enabled: stackView.currentItem ? stackView.currentItem.controllerID > 0 : false
                    controllerID: stackView.currentItem && stackView.currentItem.controllerID > 0 ? stackView.currentItem.controllerID : 0
                    serialNumber: stackView.currentItem && stackView.currentItem.currentSerialNumber > 0 ? stackView.currentItem.currentSerialNumber : 0
                    anchors.margins: ui.margin
                    Layout.fillWidth: true
                    onAddModuleClicked:  {
                        var component = Qt.createComponent("NewModuleWindow.qml");
                        if(component)
                        {
                             var win = component.createObject(mainItem);
                             win.classCode = stackView.currentItem.currentControllerClass
                             win.newModuleComplete.connect(newModuleComplete);
                             win.show();
                        }
                        else
                        {
                            console.debug("FAILED TO CREATE NewModuleWidnow");
                        }
                    }
                    onAddDeviceClicked: {
                        var component = Qt.createComponent("NewDeviceWindow.qml");
                        if(component)
                        {
                             var win = component.createObject(mainItem);
                             win.newDeviceComplete.connect(newDeviceComplete);
                             win.show();
                        }
                        else
                        {
                            console.debug("FAILED TO CREATE NewDeviceWidnow");
                        }
                    }

                    onEditClicked: {
                        showEdit(stackView.currentItem.currentIndex);
                    }
                    onEnableAddModuleChanged: {
                        console.debug("ControllerActions::onEnableAddModuleChanged + " );
                    }

                    function newModuleComplete(moduleName, classCode, cancelled)
                    {
                        if(!cancelled)
                            detailLoader.item.addNewModule(moduleName, -1, classCode);
                    }

                    function newDeviceComplete(deviceName, moduleID, moduleIndex, cancelled)
                    {
                        if(!cancelled)
                            detailLoader.item.addNewDevice(deviceName, moduleID, -1);
                    }
                }
                Loader {
                    id: detailLoader
                    anchors.margins: ui.margin
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onLoaded: {
                        binder.target = detailLoader.item;
                        console.debug("detailLoader::onLoaded");
                    }
                }
                Binding {
                    id: binder
                    property: "allowNewModule"
                    value: stackView.allowAddNew
                }
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

