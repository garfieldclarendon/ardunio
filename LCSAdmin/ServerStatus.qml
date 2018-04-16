import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import Utils 1.0

Rectangle {
    implicitHeight: textMetrics.itemHeight
    color: "#EFEEEC"
    border.color: "#DBDAD8"
    border.width: 1

    TextMetrics {
        id: textMetrics
        property int itemHeight: Math.max(addressCombo.height, textMetrics.height + 35)
        text: "CLASS>MULTI-CONTROLLER"
    }

    Component.onCompleted: {
        addressCombo.currentIndex = -1;
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
            model: api.serverList
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
