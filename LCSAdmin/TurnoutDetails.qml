import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.2
import Utils 1.0

Rectangle {
    id: main
    property var deviceEntity

    signal saveClicked();
    signal cancelClicked();
    signal showRouteDetail(var entity);

    height: 100
    width: 100

    function saveData()
    {
        console.debug("TurnoutDetails::saveData!!!!");
        deviceProperties.saveData();
    }
    MessageDialog {
        id: messageDialog
        icon: StandardIcon.Question
        standardButtons: StandardButton.Yes | StandardButton.No
        title: "New Signal Aspect"
        text: "The new Signal Aspect needs to be saved before adding conditions.  Save now?"
        visible: false
        onYes: {
            saveData();
            messageDialog.visible = false;
        }
    }

    GenericDeviceDetails {
        id: genericDetails
        devicePropertiesVisible: false
        deviceEntity: main.deviceEntity

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        onSaveClicked: {
            main.saveData();
            main.saveClicked();
        }
        onCancelClicked: {
            main.cancelClicked();
        }
    }
    DeviceProperties {
        id: deviceProperties
        deviceID: deviceEntity.data.deviceID
        deviceClass: deviceEntity.data.deviceClass

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: genericDetails.bottom
    }

    TurnoutRoutes {
        id: routeList
        deviceID: deviceEntity.data.deviceID

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: deviceProperties.bottom
        anchors.bottom: parent.bottom
        onAddClicked: {
            var routeEntity;
            routeEntity = routeList.model.getEntity(-1);
            createRouteDetailPanel(routeEntity);
        }
        onEditClicked: {
             createRouteDetailPanel(entity);
        }
    }

    function routeSaveClicked()
    {
        console.debug("routeSaveClicked()");
        var entity;
        var model;
        entity = deviceStackView.currentItem.routeEntity;

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
            routeList.model.setEntity(routeList.currentIndex, ret);
        }
        deviceStackView.pop();
    }

    function routeCancelClicked()
    {
        deviceStackView.pop();
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
        panel.width = deviceStackView.width - 10;
        panel.height = deviceStackView.height - 10;
        deviceStackView.push(panel);
    }
}
