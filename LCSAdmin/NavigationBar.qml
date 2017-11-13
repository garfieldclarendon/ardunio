import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

Rectangle {
    border.width: 1
    border.color: "lightgrey"
    height: backButton.height + 6

    property string titleText
    property bool enableBackButton: true

    signal backButtonClicked()

    GridLayout {
        anchors.fill: parent
        anchors.margins: 3

        columns: 3
        ToolButton {
            id: backButton
            enabled: enableBackButton
            contentItem: Image {
                source: "Images/navigate_left.png"
                sourceSize.width: 24
                sourceSize.height: 24
                fillMode: Image.Pad
                layer.enabled: true
                layer.effect: Desaturate {
                    desaturation: parent.enabled ? 0 : 1
                    Behavior on desaturation { NumberAnimation { easing.type: Easing.InOutQuad } }
                }
            }
            onClicked: {
                backButtonClicked();
            }
        }
        Label {
            id: title
            text: titleText
            horizontalAlignment: Qt.AlignHCenter
            color: "blue"
            font.bold: true
            font.pointSize: ui.baseFontSize + 10
            Layout.fillWidth: true
        }
        Item {
            height: backButton.height
            width: backButton.width
        }
    }
}
