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


    SignalConditionModel{
        id: signalConditionModel
    }

    header: Item{
            width: parent.width
            height: 40
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    ServerStatus {
                        id: serverStatus
                        anchors.fill: parent
                }
        }
    }
    SwipeView {
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
                                promptDeviceTypeDlg.visible = true;
                            }

                            Dialog {
                                id: promptDeviceTypeDlg
                                visible: false
                                title: "Select Device Type"
                                standardButtons: StandardButton.Ok | StandardButton.Cancel

                                ListModel {
                                    id: classModel
                                    ListElement { text: "Unknown"; classID: 0 }
                                    ListElement { text: "Turnout"; classID: 1 }
                                    ListElement { text: "Panel Input"; classID: 2 }
                                    ListElement { text: "Panel Output"; classID: 3 }
                                    ListElement { text: "Signal"; classID: 4 }
                                    ListElement { text: "Semaphore"; classID: 5 }
                                    ListElement { text: "Block"; classID: 6 }
                                }

                                GridLayout {
                                    columns: 2
                                    implicitWidth: 400
                                    implicitHeight: 100
                                    Text {
                                        text: qsTr("Class: ")
                                    }
                                    ComboBox {
                                        id: typeCombo
                                        model: classModel
                                        textRole: "text"
                                    }
                                }
                                onAccepted: {
                                    promptDeviceTypeDlg.visible = false;
                                    if(typeCombo.currentIndex > 0)
                                    {
                                        var deviceEntity = devices.model.createNewDevice(typeCombo.currentIndex);
                                        deviceEntity =
                                        createDetailPanel(typeCombo.currentIndex, deviceEntity);
                                    }
                                }
                                onDiscard: {
                                    promptDeviceTypeDlg.visible = false;
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
            panel.copyDevice.connect(copyDeviceClicked);
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
            panel.copyDevice.connect(copyDeviceClicked);
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

    function controllerCancelClicked()
    {
        controllerStackView.pop();
    }

    function copyDeviceClicked(deviceID)
    {
        console.debug("copyDetailClicked: " + deviceID);
        var entity;
        var model;
        deviceStackView.pop();
        model = deviceStackView.get(deviceStackView.index, false).model;
        entity = model.copyDevice(deviceID);
        model.setEntity(-1, entity);
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
