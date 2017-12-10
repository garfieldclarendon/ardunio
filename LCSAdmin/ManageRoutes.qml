import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    property alias model: listView.model
    property alias currentIndex: listView.currentRow

    signal editClicked(int index, var entity);
    signal addClicked();

    TextMetrics {
        id: textMetrics
//        font.family: "Courier New"
//        font.pixelSize: 25
//        font.pointSize: ui.baseFontSize
        font.bold: false
        text: "CLASS>MULTI-CONTROLLER"
    }

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
            rowDelegate: Rectangle{
                width: parent.width
                height: textMetrics.height + 10
                color: styleData.selected ? "#448" : (styleData.alternate? "#eee" : "#fff")
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
                         id: activeButton
                         enabled: styleData.value === false
                         style: ButtonStyle {
                             label: Text {
                                 enabled: activeButton.enabled
                                 font: textMetrics.font
                                 verticalAlignment: Text.AlignVCenter
                                 horizontalAlignment: Text.AlignHCenter
                                 color: enabled ? "black" : "grey"
                                 text: "Activate"
                             }
                         }
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
                    id: lockButton
                        anchors.fill: parent
                         enabled: styleData.value === undefined ? false : routeModel.data(styleData.row, "canLock") === false ? styleData.value === true : true
                         text: styleData.value === false ? "Lock" : "Unlock"
                         style: ButtonStyle {
                             label: Text {
                                 enabled: lockButton.enabled
                                 font: textMetrics.font
                                 verticalAlignment: Text.AlignVCenter
                                 horizontalAlignment: Text.AlignHCenter
                                 color: enabled ? "black" : "grey"
                                 text: lockButton.text
                             }
                         }
                         onClicked: {
                             api.lockRoute(routeModel.data(styleData.row, "routeID"), styleData.value === false);
                         }
                    }
            }
            itemDelegate: Text {
                font: textMetrics.font
                property variant value: styleData.value
                verticalAlignment: Text.AlignVCenter
                elide: styleData.elideMode
                text: styleData.value !== undefined ? styleData.value : ""
                color: parent && parent.parent.children[3].item ? (parent.parent.children[3].item.value ? "green" : "black") : "black"
                renderType: Text.NativeRendering
            }
        }
    }
}

