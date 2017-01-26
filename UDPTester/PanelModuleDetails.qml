import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    id: outerItem
    implicitHeight: 100
    implicitWidth: 100

    property int panelID: 0

    PanelInputModel {
        id: panelInputModel
        panelModuleID: panelID
    }

    PanelOutputModel {
        id: panelOutputModel
        panelModuleID: panelID
    }

     Rectangle {
       anchors.fill: parent
        border.color: "black"
        border.width: 1
       ColumnLayout {
            anchors.fill: parent

            Rectangle {
                color: "lightgrey"
                Layout.fillWidth: true
                height: 25
                Text {
                    anchors.fill: parent
                    id: topLabel
                    font.bold: true
                    font.pointSize: ui.baseFontSize + 2
                    text: qsTr("Buttons (Inputs)")
                    horizontalAlignment: Qt.AlignHCenter
                }
            }
            ListView {
                anchors.margins: ui.margin
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: inputListView
                model: panelInputModel
                delegate: inputDelegate
                spacing: ui.applyRatio(2)
                focus: true
                clip: true
            }
            Rectangle {
                color: "lightgrey"
                Layout.fillWidth: true
                height: 25
                Text {
                    anchors.fill: parent
                    id: bottomLabel
                    font.bold: true
                    font.pointSize: ui.baseFontSize + 2
                    text: qsTr("LED's (Outputs)")
                    horizontalAlignment: Qt.AlignHCenter
                }
            }

            ListView {
                anchors.margins: ui.margin
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: outputListView
                model: panelInputModel
                delegate: outputDelegate
                spacing: ui.applyRatio(2)
                focus: true
                clip: true
            }
        }

       Component {
           id: inputDelegate
            GroupBox {
                width: outerItem.width
                height: 60

            }
       }

       Component {
           id: outputDelegate
            GroupBox {
                width: outerItem.width
                height: 60

            }
       }
    }
}
