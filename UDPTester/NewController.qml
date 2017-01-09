import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Rectangle {
    border.color: "lightgrey"
    border.width: 1

    property string serialNumber: ""
    property int classID: 0

    signal addController();
    signal editFinished(bool save);
    signal replaceController(string serialNumber, int classID);

    Rectangle {
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: ui.margin
            spacing: ui.margin

            GridLayout {
                id: grid
                anchors.margins: ui.margin
                anchors.fill: parent
                columns: 2

                Text { text: "Serial #:"; font.bold: true  }
                TextField { id:serialNumberText; text: serialNumber; Layout.fillWidth: true; }
                Text { text: "Class:"; font.bold: true  }
                ComboBox
                {
                    id: classIDCombo
                    currentIndex: classID
                    model: ["Unknown", "Turnout", "Panel", "Route", "Signal", "Block", "System", "Application"]
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                spacing: ui.margin
                anchors.margins: ui.margin

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: newControllerButton
                    text: "New"

                    onClicked: {
                        addController();
                    }
                }

                Button {
                    id: replaceControllerButton
                    text: "Replace"

                    onClicked: {
                        replaceController(serialNumber, classID);
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
        }
    }
}

