import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import Utils 1.0

Item {
    property string pathName: "Devices"
    property string title: "Devices"
    property alias model: deviceList.model
    property alias currentIndex:deviceList.currentIndex

    signal addClicked();
    signal updateClicked(int index);
    signal deleteClicked(int Index);

    ControllerModuleModel {
        id: moduleModel
        onModelReset: {
            deviceTypeCombo.currentIndex = -1;
        }
    }

    GridLayout {
        columns: 4
        anchors.fill: parent

        TextField {
            id: searchText
            placeholderText: "Enter Search Text"
            text: deviceList.model.filterText
            inputMethodHints: Qt.ImhNoPredictiveText
            Layout.fillWidth: true
            onTextChanged: {
                if(text && text.length > 0)
                {
                    deviceList.currentIndex = -1;
                    deviceTypeCombo.currentIndex = -1;
                }
                deviceList.model.filterText = searchText.text;
            }
        }

        Text {
            id: name
            horizontalAlignment: Text.AlignRight
            text: qsTr("Filter By:")
            font.bold: true
            Layout.fillWidth: true
        }
        ComboBox {
            id: deviceTypeCombo
            Layout.fillWidth: true
            model: moduleModel
            currentIndex: -1
            textRole: "moduleName"
            onCurrentIndexChanged: {
                searchText.text = "";
                deviceList.currentIndex = -1;
                if(deviceTypeCombo.currentIndex >= 0 && moduleModel.data(deviceTypeCombo.currentIndex, "controllerModuleID"))
                    deviceList.model.controllerModuleID =  moduleModel.data(deviceTypeCombo.currentIndex, "controllerModuleID");
                else
                    deviceList.model.controllerModuleID = 0;
            }
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
                crudButtons.enableUpdate = true;
                deleteClicked(deviceList.currentIndex);
            }
            onUpdateButtonClicked:  {
                updateClicked(deviceList.currentIndex);
                crudButtons.enableUpdate = false;
            }
        }

        DeviceList {
            id: deviceList
            Layout.columnSpan: 4
            Layout.fillHeight: true
            Layout.fillWidth: true

            onModelChanged: {
                crudButtons.enableUpdate = deviceList.model.modelChanged;
            }

            onItemDoubleClicked: {
                navigationBar.titleText = "Device Details";
                console.debug("createDetailPanel");
                var component;
                var panel
                var entity = deviceList.model.getEntity(deviceList.currentIndex);
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
        }
    }
}
