import QtQuick 2.0

Item {
    property string pathName: "Devices"
    property string title: "Devices"

    DeviceList {
        id: deviceList
        anchors.fill: parent

        onItemDoubleClicked: {
            navigationBar.titleText = "Device Details";
            var component;
            var panel
            var entity = deviceList.model.getEntity(deviceList.currentIndex);
            component = Qt.createComponent("GenericDeviceDetails.qml");
            panel = component.createObject(deviceStackView, {"deviceEntity": entity});
            panel.saveClicked.connect(detailSaveClicked);
            panel.cancelClicked.connect(detailCancelClicked);
            panel.width = deviceStackView.width - 10;
            panel.height = deviceStackView.height - 10;
            deviceStackView.push(panel);
        }
    }
}
