import QtQuick 2.0
import QtQuick.Layouts 1.3

Item {
    property string pathName: "Devices"
    property string title: "Devices"

    GridLayout {
        columns: 2
        anchors.fill: parent
        Item {
            id:placeholder
            Layout.fillWidth:  true
        }

        CRUDButtons {
            id: crudButtons
            enableAdd: api.apiReady
            enableDelete: deviceList.currentIndex >= 0 ? (api.apiReady ? true : false) : false

            onAddButtonClicked: {
                console.debug("CRUDButtons::onAddClicked ");
                addClicked(deviceList.model.data(deviceList.currentIndex, "deviceClass"));
            }
            onDeleteButtonClicked: {
                deleteClicked(deviceList.currentIndex);
            }
            onUpdateButtonClicked:  {
                updateClicked(deviceList.model.data(deviceList.currentIndex, "deviceClass"), deviceList.currentIndex);
            }
        }

        DeviceList {
            id: deviceList
            Layout.columnSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true

            onModelChanged: {
                crudButtons.enableUpdate = deviceList.model.modelChanged;
            }

            onItemDoubleClicked: {
                navigationBar.titleText = "Device Details";
                var component;
                var panel
                var entity = deviceList.model.getEntity(deviceList.currentIndex);
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
    }
}
