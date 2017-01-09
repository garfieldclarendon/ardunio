import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import Utils 1.0

Item {
    PanelOutputModel {
        id: outputModel
        panelModuleID: id
    }

    PanelInputModel {
        id: inputModel
        panelModuleID: id
    }

    Text
    {
        text: "Inputs"
        width: parent.width
        font.bold: true
        font.pointSize: 10
        anchors.margins: 5
        color: "blue"
        horizontalAlignment: Text.AlignHCenter
    }

    ListView {
        id: inputList
        width: parent.width
        delegate: PanelInputDelegate {}
        model: inputModel
        Layout.fillHeight: true
        clip: true
    }


    Text
    {
        text: "Outputs"
        font.bold: true
        width: parent.width
        font.pointSize: 10
        anchors.margins: 5
        color: "blue"
        horizontalAlignment: Text.AlignHCenter
    }

    ListView {
        id: outputList
        width: parent.width
        delegate: PanelOutputDelegate {}
        model: outputModel
        Layout.fillHeight: true
        clip: true
    }
}

