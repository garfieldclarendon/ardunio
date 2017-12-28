import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Utils 1.0

GroupBox {
    id: devicePropertyGroup
    title: "Additional Information"
    property int deviceID: -1
    property int deviceClass: 0

    DevicePropertyModel {
        id: model
        deviceID:  devicePropertyGroup.deviceID
        deviceClass: devicePropertyGroup.deviceClass
    }

    ListView {
        anchors.fill: parent
        clip: true

        Component {
            id: detialDelegate
            RowLayout {
                id: wrapper
                width: parent.width
                height: valueField.height + 12
                Text {
                    id: keyText
                    text: "<b>" + key + ":</b>"
                    horizontalAlignment: Text.AlignRight
                    Layout.fillWidth: true
                }
                TextField {
                    id: valueField
                    text: value ? value : ""
                    onTextChanged: {
                        value = valueField.text
                    }
                }
            }
        }

        model: model
        delegate: detialDelegate
        focus: true
    }

    function saveData()
    {
        model.save();
    }
}
