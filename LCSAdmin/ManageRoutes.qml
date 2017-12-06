import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property alias model: listView.model
    property alias currentIndex: listView.currentRow

    signal editClicked(int index, var entity);
    signal addClicked();

    RouteModel {
        id: routeModel
    }

    GridLayout {
        anchors.fill: parent
        anchors.margins: 5
        columns: 3

        Item {
            height: 10
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }
        CRUDButtons {
            id: buttons
            enableAdd: api.apiReady
            enableDelete: listView.currentRow >= 0 ? (api.apiReady ? true : false) : false

            onAddButtonClicked: {
                console.debug("CRUDButtons::onAddClicked ");
                addClicked();
            }
            onDeleteButtonClicked: {
                routeModel.deleteRow(listView.currentRow);
                enableUpdate = true;
            }
            onUpdateButtonClicked:  {
                routeModel.save();
                enableUpdate = false;
            }
        }

        TableView {
            id: listView
            model: routeModel
            focus: true
            clip: true
            Layout.columnSpan: 3
            Layout.fillHeight: true
            Layout.fillWidth: true
            onClicked: {
                listView.currentRow = row
            }
            onDoubleClicked: {
                var ent = routeModel.getEntity(row);
                editClicked(row, ent);
            }
            TableViewColumn {
                role: "routeID"
                title: "ID"
                width: 50
            }
            TableViewColumn {
                role: "routeName"
                title: "Name"
                width: 150
            }
            TableViewColumn {
                role: "routeDescription"
                title: "Description"
                width: 150
            }
            TableViewColumn {
                role: "isActive"
                title: "Active?"
                width: 90
            }
            TableViewColumn {
                role: "isLocked"
                title: "Locked?"
                width: 90
            }
            TableViewColumn {
                role: "isActive"
                title: ""
                width: 130
                delegate:
                    Button {

                         text: "Activate"
                         enabled: styleData.value === false
                         onClicked: {
                             api.activateRoute(routeModel.data(styleData.row, "routeID"));
                         }
                    }
            }
            TableViewColumn {
                role: "isLocked"
                title: ""
                width: 130
                delegate:
                    Button {
                         enabled: styleData.value ? routeModel.data(styleData.row, "canLock") === false : 0
                         text: styleData.value === false ? "Lock" : "Unlock"
                         onClicked: {
                             api.lockRoute(routeModel.data(styleData.row, "routeID"), styleData.value === false);
                         }
                    }
            }
            itemDelegate: Text {
                property variant value: styleData.value
                elide: styleData.elideMode
                text: styleData.value !== undefined ? styleData.value : ""
                color: parent && parent.parent.children[3].item ? (parent.parent.children[3].item.value ? "green" : "black") : "black"
                renderType: Text.NativeRendering
            }
        }
    }
}

