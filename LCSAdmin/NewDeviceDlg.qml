import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Dialog {
    id: newDeviceDlg
    visible: false
    title: "New Device"
    standardButtons: StandardButton.Ok | StandardButton.Cancel
    width: 400
    height: 400

    property alias deviceName: nameEdit.text
    property alias deviceDescription: descEdit.text
    property alias deviceClass: typeCombo.currentIndex

    onVisibleChanged: {
        if(visible)
        {
            nameEdit.text = "";
            descEdit.text = "";
        }
    }

    ListModel {
        id: classModel
        ListElement { text: "Unknown"; classID: 0 }
        ListElement { text: "Turnout"; classID: 1 }
        ListElement { text: "Panel Input"; classID: 2 }
        ListElement { text: "Panel Output"; classID: 3 }
        ListElement { text: "Signal"; classID: 4 }
        ListElement { text: "Semaphore"; classID: 5 }
        ListElement { text: "Block"; classID: 6 }
    }

    GridLayout {
        anchors.fill: parent
        columns: 2
        Text {
            text: qsTr("Class: ")
        }
        ComboBox {
            id: typeCombo
            model: classModel
            textRole: "text"
        }
        Text {
            text: qsTr("Name: ")
        }
        TextField {
            id: nameEdit
        }
        Text {
            text: qsTr("Description: ")
        }
        TextField {
            id: descEdit
        }
    }
    onDiscard: {
        newDeviceDlg.visible = false;
    }
}
