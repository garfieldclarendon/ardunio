import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import Utils 1.0

Rectangle {
    height: serverAddress.height + 10
    color: "#EFEEEC"
    border.color: "#DBDAD8"
    border.width: 1

    Component.onCompleted: {
        addressCombo.currentIndex = -1;
        serverListModel.append({"text": "127.0.0.1"});
    }

    function apiAddressChanged()
    {
//        var entry = "{ text: \"" + api.serverAddress + "\" }";
//        serverListModel.append(entry);
//        addressCombo.currentIndex = serverListModel.count - 1;
    }

    ListModel {
        id: serverListModel
    }

    GridLayout {
        anchors.margins: 5
        anchors.fill: parent
        columns: 5
        columnSpacing: 5
        Text {
            id: serverAddress
            text: "<b>Server:</b>"
        }
        ComboBox {
            id: addressCombo
            editable: true
            model: serverListModel
            currentIndex: -1
            editText: api.serverAddress
            onCurrentIndexChanged: {
                api.serverAddress = addressCombo.currentText
            }
        }
        Text {
            id: serverPort
            text: "<b>Port: </b>" +api.serverPort
        }

        Item {
            Layout.fillWidth: true
        }
        Text {
            id: statusText
            text: api.apiReady ? "<b>ONLINE</b>" : "<b>OFFLINE</b>"
            color: api.apiReady ? "blue" : "red"
        }
    }
}
