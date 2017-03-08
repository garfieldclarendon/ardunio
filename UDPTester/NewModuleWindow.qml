import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4

Window {
    id: newModulePrompt
    property int classCode
    property string name
    property bool cancelled: false

    signal newModuleComplete(string moduleName, int classCode, bool cancelled)

    color: systemPalette.window
    flags: Qt.Dialog
    modality: Qt.ApplicationModal
    title: "Add New Module"

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
                text: "Class:"
                font.bold:  true
            }
            ComboBox {
                id: classCombo
                model: classModel
                textRole: "name"
                currentIndex: classCode
                Layout.fillWidth:  true
            }
            Text {
                text: "Name:"
                font.bold:  true
            }
            TextEdit {
                id: nameEdit
                text: name
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
                   newModuleComplete(nameEdit.text, classCombo.currentIndex, cancelled);
                    newModulePrompt.close();
                }
            }
            Button {
                id: cancel
                text: "Cancel"
                onClicked: {
                    cancelled = true;
                    newModuleComplete(name, classCode, cancelled);
                    newModulePrompt.close();
                }
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }

    ListModel {
        id: classModel
        ListElement
        {
            name: "Unknown"
            classID: 0
        }
        ListElement
        {
            name: "Turnout"
            classID: 1
        }
        ListElement
        {
            name: "Panel"
            classID: 2
        }
        ListElement
        {
            name: "Route"
            classID: 3
        }
        ListElement
        {
            name: "Signal"
            classID: 4
        }
        ListElement
        {
            name: "Semaphore"
            classID: 5
        }
        ListElement
        {
            name: "Block"
            classID: 6
        }
        ListElement
        {
            name: "Multi-Module"
            classID: 7
        }
        ListElement
        {
            name: "System"
            classID: 8
        }
        ListElement
        {
            name: "Application"
            classID: 9
        }
        ListElement
        {
            name: "Unknown"
            classID: 0
        }
    }
}

