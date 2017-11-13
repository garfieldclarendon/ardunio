import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

Rectangle {
    property bool enableAdd: false
    property bool enableUpdate: false
    property bool enableDelete: false

    signal addButtonClicked()
    signal updateButtonClicked()
    signal deleteButtonClicked()

    border.width: 1
    border.color: "lightgrey"
    height: addButton.height + 10
    width: (addButton.width + 4) * 3 + 3

    GridLayout {
        anchors.fill: parent
        anchors.margins: 2
        columns: 3
        ToolButton {
            id: addButton
            contentItem: Image {
                source: "Images/add.png"
                sourceSize.width: 24
                sourceSize.height: 24
                fillMode: Image.Pad
                layer.enabled: true
                layer.effect: Desaturate {
                    desaturation: parent.enabled ? 0 : 1
                    Behavior on desaturation { NumberAnimation { easing.type: Easing.InOutQuad } }
                }
            }
            enabled: enableAdd
            onClicked: {
                addButtonClicked();
            }
        }
        ToolButton {
            id: updateButton
            contentItem: Image {
                source: "Images/save.png"
                sourceSize.width: 24
                sourceSize.height: 24
                fillMode: Image.Pad
                layer.enabled: true
                layer.effect: Desaturate {
                    desaturation: parent.enabled ? 0 : 1
                    Behavior on desaturation { NumberAnimation { easing.type: Easing.InOutQuad } }
                }
            }
            enabled: enableUpdate
            onClicked: {
                updateButtonClicked();
            }
        }
        ToolButton {
            id: deleteButton
            contentItem: Image {
                source: "Images/delete.png"
                sourceSize.width: 24
                sourceSize.height: 24
                fillMode: Image.Pad
                layer.enabled: true
                layer.effect: Desaturate {
                    desaturation: parent.enabled ? 0 : 1
                    Behavior on desaturation { NumberAnimation { easing.type: Easing.InOutQuad } }
                }
            }
            enabled: enableDelete
            onClicked: {
                deleteButtonClicked();
            }
        }
    }
}
