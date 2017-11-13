import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import Utils 1.0


Rectangle {
    id: mainRect
    property var newEntity
    property var controllerModel

    signal saveClicked();
    signal cancelClicked();

    implicitHeight: 400
    width: 400
    clip: true

    //    border.color: "red"
    //    border.width: 1

    GridLayout {
        anchors.fill: parent
        columns: 3
        Label {
            id: comboLabel
            text: "<b>Replace Controller: </b>"
        }
        ComboBox {
            id: controllerCombo
            textRole: "controllerName"
            model: controllerModel
            currentIndex: -1
            Layout.fillWidth: true
        }
        Button {
            id: replaceButton
            text: "Replace"
            enabled: !(controllerCombo.currentIndex === -1)
            onClicked: {
                var e = controllerModel.getEntity(controllerCombo.currentIndex);
                e.setValue("serialNumber", newEntity.data.serialNumber);
                controllerModel.setEntity(controllerCombo.currentIndex, e);
                controllerModel.save();
                api.restartController(newEntity.data.serialNumber);
                // Since the data is already saved, we can just call cancel
                // to close this form.  Otherwise, saveClicked() will try to save
                // the changes again.
                cancelClicked();
            }
        }

        GroupBox {
            title: "<b>New Controller Details</b>"
            ControllerDetails {
                id: controllerDetails
                anchors.fill: parent
                controllerEntity: newEntity
                onSaveClicked: saveClicked()
                onCancelClicked: cancelClicked()
            }

            Layout.columnSpan: 3
            Layout.fillHeight: true
            Layout.fillWidth: true
            OpacityAnimator on opacity{
                from: 1;
                to: 0;
                duration: 500
                running: (controllerCombo.currentIndex !== -1)
            }
        }
    }
}
