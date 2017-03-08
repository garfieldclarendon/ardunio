import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4

Window {
    id: newDevicePrompt
    property int moduleID
    property int moduleIndex
    property string name
    property string description

    property bool cancelled: false

    signal newDeviceComplete(string deviceName, string description, int moduleID, int moduleIndex, bool cancelled)

    color: systemPalette.window
    flags: Qt.Dialog
    modality: Qt.ApplicationModal
    title: "Add New Device"

    width: 400
    height: 400

    SystemPalette {
        id: systemPalette
    }

    ColumnLayout
    {
        anchors.margins: 10
        anchors.fill: parent
        GridLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            columns: 2
            Text {
                text: "Name:"
                font.bold:  true
            }
            TextEdit {
                id: nameEdit
                text: name
                Layout.fillWidth:  true
            }
            Text {
                text: "Description:"
                font.bold:  true
            }
            TextEdit {
                id: descriptionEdit
                text: description
                Layout.fillWidth:  true
            }
            Text {
                text: "Index:"
                font.bold:  true
            }
            TextEdit {
                id: indexEdit
                text: moduleIndex
                Layout.fillWidth:  true
            }
            Item {
                Layout.fillHeight: true
            }
            Item {
                Layout.fillHeight: true
            }
        }
        RowLayout {
            Item {
                Layout.fillWidth: true
            }
            Button {
                id: close
                text: "Done"
               onClicked: {
                   cancelled = false;
                   newDeviceComplete(nameEdit.text, descriptionEdit.text, moduleID, indexEdit.text, cancelled);
                    newDevicePrompt.close();
                }
            }
            Button {
                id: cancel
                text: "Cancel"
                onClicked: {
                    cancelled = true;
                    newDeviceComplete(nameEdit.text, descriptionEdit.text, moduleID, indexEdit.text, cancelled);
                    newDevicePrompt.close();
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }
}

