import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property int selectedControllerID: 0

    signal controllerSelected(int controllerID);

    ControllerModel {
        id: model
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: ui.margin
        RowLayout {
            anchors.margins: ui.margin
            Layout.fillWidth: true
            Text {
                text: "Search: "
            }
            TextEdit {
                id: searchEdit
                Layout.fillWidth: true

                onTextChanged: {
                    if(searchEdit.text.length > 0)
                        model.filterByText(searchEdit.text);
                    else
                        model.clearFilter();
                }
            }
        }

        TableView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: listView
            model: model
            focus: true

            TableViewColumn {
                    role: "id"
                    title: "ID"
                    width: 100
                }
            TableViewColumn {
                role: "controllerName"
                title: "Name"
                width: 200
            }
            TableViewColumn {
                role: "controllerDescription"
                title: "Name"
                width: 200
            }
            TableViewColumn {
                role: "serialNumber"
                title: "Serial #"
                width: 200
            }

            onDoubleClicked: {
                var controllerID = model.getData(listView.currentRow, "id");
                controllerSelected(controllerID);
            }
        }
        RowLayout {
            anchors.margins: ui.margin
            Item {
                Layout.fillWidth: true
            }

            Button {
                id: cancelButton
                text: "Cancel"
                onClicked: {
                    controllerSelected(-1)
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}

