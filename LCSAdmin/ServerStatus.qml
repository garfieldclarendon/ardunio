import QtQuick 2.0
import QtQuick.Layouts 1.3
import Utils 1.0

Rectangle {
    height: serverAddress.height + 10
    color: "#EFEEEC"
    border.color: "#DBDAD8"
    border.width: 1
    GridLayout {
        anchors.margins: 5
        anchors.fill: parent
        columns: 4
        columnSpacing: 5
        Text {
            id: serverAddress
            text: "<b>Server:</b> " + api.serverAddress
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
