import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property var editModel
    property int index: -1
    property int serialNumber: 0
    property int classID: 0

    signal editFinished(bool save);

    Rectangle {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: ui.margin
            spacing: ui.margin

            GridLayout {
                id: grid
                anchors.margins: ui.margin
                columns: 2

                Text { text: "Name:"; font.bold: true; }
                TextField { id:name; text: editModel.getData(index, "controllerName"); Layout.fillWidth: true; }
                Text { text: "Description:"; font.bold: true  }
                TextField { id:description; text: editModel.getData(index, "controllerDescription"); Layout.fillWidth: true; }
                Text { text: "Class:"; font.bold: true  }
                ComboBox
                {
                    id: classID
                    currentIndex: classID > 0 ? classID : editModel.getData(index, "controllerClass")
                    model: ["Unknown", "Turnout", "Panel", "Route", "Signal", "Semaphore", "Block", "System", "Application"]
                    Layout.fillWidth: true
                }
                Text { text: "Serial #:"; font.bold: true  }
                TextField { id:serialNumber; text: editModel.getData(index, "serialNumber"); Layout.fillWidth: true; }
            }

            RowLayout {
                spacing: ui.margin
                anchors.margins: ui.margin

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: finishedButton
                    text: "Save"

                    onClicked: {
                        editModel.setData(index, "controllerName", name.text);
                        editModel.setData(index, "controllerDescription", description.text);
                        editModel.setData(index, "serialNumber", serialNumber.text);
                        editModel.setData(index, "controllerClass", classID.currentIndex);
                        editFinished(true);
                    }
                }

                Button {
                    id: cancelButton
                    text: "Cancel"

                    onClicked: {
                        editFinished(false);
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.fillHeight: true
            }

            Component.onCompleted: {
                classID.currentIndex = editModel.getData(index, "controllerClass");
            }
        }
    }
}

