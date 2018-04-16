import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import Utils 1.0

ApplicationWindow {
    visible: true
    width: 840
    height: 768
    title: qsTr("Layoutout Control System - Administration")

    BusyIndicator {
        anchors.centerIn: parent
        running: api.busy
        z: 100
    }

    SignalConditionModel{
        id: signalConditionModel
    }

    header: Item{
            width: parent.width
            height: serverStatus.implicitHeight
                ServerStatus {
                    id: serverStatus
                    anchors.fill: parent
                    anchors.margins: 5
        }
    }
    StackLayout {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        Page {
            id: devicePage
            GridLayout {
                columns: 1
                anchors.fill: parent
                anchors.margins: 5
                NavigationBar {
                    Layout.fillWidth: true
                    id: navigationBar
                    onBackButtonClicked: {
                        if(deviceStackView.depth > 1)
                            deviceStackView.pop();
                    }
                    enableBackButton: deviceStackView.depth > 1 ? true : false
                }

                StackView {
                    id: deviceStackView
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onCurrentItemChanged: {
                        if(deviceStackView.currentItem.title)
                            navigationBar.titleText = deviceStackView.currentItem.title
                        else
                            navigationBar.titleText = "";
//                        if(depth == 1)
//                            navigationBar.titleText = "";
                    }

                    onWidthChanged: {
                        if(currentItem)
                            currentItem.width = deviceStackView.width - 10
                    }
                    onHeightChanged: {
                        if(currentItem)
                            currentItem.width = deviceStackView.Height - 10
                    }

                    Component {
                        id: list
                        DeviceListPage {
                            id: devices
                            onAddClicked: {
                                newDeviceDlg.visible = true;
                            }
                            onDeleteClicked: {
                                devices.model.deleteRow(devices.currentIndex);

                            }
                            onUpdateClicked: {
                                devices.model.save();
                            }

                            NewDeviceDlg {
                                id: newDeviceDlg
                                onAccepted: {
                                    newDeviceDlg.visible = false;
                                    var deviceEntity = devices.model.createNewDevice(newDeviceDlg.deviceName, newDeviceDlg.deviceDescription, newDeviceDlg.deviceClass, true);
                                    createDetailPanel(newDeviceDlg.deviceClass, deviceEntity);
                                }
                                onDiscard: {
                                    newDeviceDlg.visible = false;
                                }
                            }
                        }
                        }

                    Component.onCompleted: {
                        deviceStackView.push(list)
                    }
                } //StackView
            } //GridLayout
        } //Page
        /////////////////////////////////////
        Page {
            id: controllerPage
            GridLayout {
                columns: 1
                anchors.fill: parent
                anchors.margins: 5
                NavigationBar {
                    Layout.fillWidth: true
                    id: controllerNavigationBar
                    onBackButtonClicked: {
                        if(controllerStackView.depth > 1)
                            controllerStackView.pop();
                    }
                    enableBackButton: controllerStackView.depth > 1 ? true : false
                }

                StackView {
                    id: controllerStackView
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onCurrentItemChanged: {
                        if(depth == 1)
                            controllerNavigationBar.titleText = "";
                    }

                    onWidthChanged: {
                        if(currentItem)
                            currentItem.width = controllerStackView.width - 10
                    }
                    onHeightChanged: {
                        if(currentItem)
                            currentItem.width = controllerStackView.Height - 10
                    }

                    Component {
                        id: controllerList
                        ManageControllers {
                            id: controllers
                            onAddClicked: {
                                var controllerEntity;
                                controllerEntity = controllers.model.getEntity(-1);
                                createControllerDetailPanel(controllerClass, controllerEntity);
                            }
                            onEditClicked: {
                                 createControllerDetailPanel(controllerClass, entity);
                            }
                            onNewController: {
                                console.debug("NEW CONTROLLER: " + newSerialNumber);
                                var component;
                                var panel
                                var controllerEntity;
                                controllerEntity = controllers.model.getEntity(-1);
                                controllerEntity.data.serialNumber = newSerialNumber;
                                controllerNavigationBar.titleText = "New Controller: " + controllerEntity.data.serialNumber;
                                component = Qt.createComponent("NewController.qml");
                                panel = component.createObject(controllerStackView, {"newEntity": controllerEntity, "controllerModel": controllers.model});
                                panel.saveClicked.connect(controllerSaveClicked);
                                panel.cancelClicked.connect(controllerCancelClicked);
                                panel.width = controllerStackView.width - 10;
                                panel.height = controllerStackView.height - 10;
                                controllerStackView.push(panel);
                            }
                        }
                    }

                    Component.onCompleted: {
                        controllerStackView.push(controllerList)
                    }
                } //StackView
            } //GridLayout
        } //Page
        /////////////////////////////////////
        Page {
            id: routePage
            GridLayout {
                columns: 1
                anchors.fill: parent
                anchors.margins: 5
                NavigationBar {
                    Layout.fillWidth: true
                    id: routeNavigationBar
                    onBackButtonClicked: {
                        if(routeStackView.depth > 1)
                            routeStackView.pop();
                    }
                    enableBackButton: routeStackView.depth > 1 ? true : false
                }

                StackView {
                    id: routeStackView
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onCurrentItemChanged: {
                        if(depth == 1)
                            routeNavigationBar.titleText = "";
                    }

                    onWidthChanged: {
                        if(currentItem)
                            currentItem.width = routeStackView.width - 10
                    }
                    onHeightChanged: {
                        if(currentItem)
                            currentItem.width = routeStackView.Height - 10
                    }

                    Component {
                        id: routeList
                        ManageRoutes {
                            id: routes
                            onAddClicked: {
                                var routeEntity;
                                routeEntity = routes.model.getEntity(-1);
                                createRouteDetailPanel(routeEntity);
                            }
                            onEditClicked: {
                                 createRouteDetailPanel(entity);
                            }
                        }
                    }

                    Component.onCompleted: {
                        routeStackView.push(routeList)
                    }
                } //StackView
            } //GridLayout
        } //Page
    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        TabButton {
            text: qsTr("Devices")
        }
        TabButton {
            text: qsTr("Controllers")
        }
        TabButton {
            text: qsTr("Routes")
        }
    }

    function createRouteDetailPanel(entity)
    {
        var component;
        var panel

        routeNavigationBar.titleText = "Route Details";
        component = Qt.createComponent("RouteDetails.qml");
        panel = component.createObject(routeStackView, {"routeEntity": entity});
        panel.saveClicked.connect(routeSaveClicked);
        panel.cancelClicked.connect(routeCancelClicked);
        panel.width = routeStackView.width - 10;
        panel.height = routeStackView.height - 10;
        routeStackView.push(panel);
    }

    function createControllerDetailPanel(controllerClass, entity)
    {
        var component;
        var panel
        controllerNavigationBar.titleText = "Controller Details";
        component = Qt.createComponent("ControllerDetails.qml");
        panel = component.createObject(controllerStackView, {"controllerEntity": entity});
        panel.saveClicked.connect(controllerSaveClicked);
        panel.cancelClicked.connect(controllerCancelClicked);
        panel.itemDoubleClicked.connect(controllerModuleDoubleClicked);
        panel.width = controllerStackView.width - 10;
        panel.height = controllerStackView.height - 10;
        controllerStackView.push(panel);
    }

    function createDetailPanel(deviceClass, entity)
    {
        console.debug("createDetailPanel");
        var component;
        var panel
        if(deviceClass === 4 || deviceClass === 5)
        {
            navigationBar.titleText = "Signal Details";
            component = Qt.createComponent("SignalDetails.qml");
            panel = component.createObject(deviceStackView, {"deviceEntity": entity});
            panel.saveClicked.connect(detailSaveClicked);
            panel.cancelClicked.connect(detailCancelClicked);
            panel.aspectDoubleClicked.connect(showAspectConditions);
            panel.width = deviceStackView.width - 10;
            panel.height = deviceStackView.height - 10;
            deviceStackView.push(panel);
        }
        else
        {
            navigationBar.titleText = "Device Details";
            component = Qt.createComponent("GenericDeviceDetails.qml");
            panel = component.createObject(deviceStackView, {"deviceEntity": entity});
            panel.saveClicked.connect(detailSaveClicked);
            panel.cancelClicked.connect(detailCancelClicked);
            panel.width = deviceStackView.width - 10;
            panel.height = deviceStackView.height - 10;
            deviceStackView.push(panel);
        }
    }
    MessageDialog {
        id: messageDialog
        title: "Save Error"
        text: "Error Saving: "
        onAccepted: {
            messageDialog.visible = false;
        }
    }
    function routeSaveClicked()
    {
        console.debug("routeSaveClicked()");
        var entity;
        var model;
        entity = routeStackView.currentItem.routeEntity;
        routeStackView.pop();

        model = routeStackView.currentItem.model;
        var ret;
        if(entity.getValue("routeID") > 0)
            ret = api.saveEntity(entity);
        else
            ret = api.saveEntity(entity, true);
        if(ret.hasError())
        {
            console.debug("ERROR SAVING!!!!!! " + ret.errorText());
            messageDialog.text = "Error saving: " + ret.errorText();
            messageDialog.visible = true;
        }
        else
        {
            model.setEntity(routeStackView.currentItem.currentIndex, entity);
        }
    }

    function routeCancelClicked()
    {
        routeStackView.pop();
    }
    function controllerSaveClicked()
    {
        console.debug("controllerSaveClicked()");
        var entity;
        var model;
        entity = controllerStackView.currentItem.controllerEntity;
        controllerStackView.pop();

        model = controllerStackView.currentItem.model;
        var ret;
        ret = api.saveEntity(entity, entity.data.controllerID < 1);
        if(ret.hasError())
        {
            console.debug("ERROR SAVING!!!!!! " + ret.errorText());
            messageDialog.text = "Error saving: " + ret.errorText();
            messageDialog.visible = true;
        }
        else
        {
            model.setEntity(controllerStackView.currentItem.currentIndex, entity);
        }
    }

    function controllerModuleDoubleClicked(moduleID)
    {
        console.debug("controllerModuleDoubleClicked!");
        var component = Qt.createComponent("ModuleDevicesPage.qml");
        var panel = component.createObject(deviceStackView, { "moduleID": moduleID, "height": deviceStackView.height - 10, "width": deviceStackView.width - 10 });
//        panel.closeButtonClicked.connect(detailCancelClicked);
        controllerStackView.push(panel);
        console.debug("controllerModuleDoubleClicked! DONE");
    }

    function controllerCancelClicked()
    {
        controllerStackView.pop();
    }

    function detailSaveClicked()
    {
        console.debug("detailSaveClicked()");
        var entity;
        var model;
        entity = deviceStackView.currentItem.deviceEntity;
        deviceStackView.pop();

        model = deviceStackView.get(deviceStackView.index, false).model;
        var ret;
        ret = api.saveEntity(entity, entity.data.deviceID < 1);
        if(ret.hasError())
        {
            console.debug("ERROR SAVING!!!!!! " + ret.errorText());
            messageDialog.text = "Error saving: " + ret.errorText();
            messageDialog.visible = true;
        }
        else
        {
            model.setEntity(deviceStackView.get(deviceStackView.index, false).currentIndex, entity);
            signalConditionModel.save();
        }
    }

    function detailCancelClicked()
    {
        deviceStackView.pop();
    }

    function showAspectConditions(aspectID)
    {
        signalConditionModel.aspectID = aspectID;
        var component = Qt.createComponent("SignalConditionDetails.qml");
        var panel = component.createObject(deviceStackView, { "conditionModel": signalConditionModel, "height": deviceStackView.height - 10, "width": deviceStackView.width - 10 });
        panel.closeButtonClicked.connect(detailCancelClicked);
        deviceStackView.push(panel);
    }
}
