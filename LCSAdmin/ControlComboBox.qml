import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import Utils 1.0

Rectangle {
    id: comboBox1
    height: layout.height

    property string dataValue
    property string lookupField
    property string displayField
    property var model
    property int currentIndex: -1

    onCurrentIndexChanged: {

    }

    TextMetrics {
        id: textMetrics
        font: label1.font
        text: "SAMPLE TEXT"
    }

    RowLayout
    {
        id: layout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        TextField {
            id: label1
            readOnly: true
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
        }
    }
    Image {
        id: navigationImage
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        source: "Images/arrow_down.png"
        sourceSize.height: 24
        sourceSize.width: 24
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.debug("ComboBox::mouse clicked!");
            popup1.open();
        }
    }

    function modelLoadComplete()
    {
        table1.currentIndex = currentIndex;
        label1.text = model.data(currentIndex, displayField) ? model.data(currentIndex, displayField): "";
    }

    Component.onCompleted: {
        modelLoadComplete();
    }

    Dialog {
        id: popup1
        visible: false

        standardButtons: StandardButton.Save | StandardButton.Cancel

        property alias currentRow: table1.currentIndex

        onAccepted: {
            console.debug("onAccepted: " + contentItem.selectedIndex);
            label1.text = comboBox1.model.data(contentItem.selectedIndex, displayField);
            dataValue = comboBox1.model.data(contentItem.selectedIndex, lookupField);
            console.debug("onAccepted: " + dataValue);
            filterText.text = "";
        }

        onRejected: {
            filterText.text = "";
        }

        contentItem: Rectangle {
            id: contentRect
            property var lookupModel: comboBox1.model
            property alias selectedIndex: table1.currentIndex
            color: "white"
            implicitHeight: Screen.height < 600 ? Screen.height - 80 : 600
            implicitWidth: Screen.width > 600 ? 600 : Screen.width - 15


            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 5
                TextField {
                    Layout.fillWidth: true
                    id: filterText
                    placeholderText: "Search"
                    text: model.filterText
                    onTextEdited: {
                        model.filterText = filterText.text;
                    }
                }

                ListView {
                    id: table1
                    model: contentRect.lookupModel
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    spacing: 1

                    clip: true

                    delegate: Rectangle {
                        border.color: index === table1.currentIndex ? "lightgrey" : "white"
                        border.width: 1
                        height: rowText.height + 15
                        width: parent.width

                        Text {
                            id: rowText
                            anchors.right: parent.right
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.margins: 5
                            text: contentRect.lookupModel.data(index, displayField) ? contentRect.lookupModel.data(index, displayField) : ""

                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            onClicked: {
                                console.debug("ComboBox::row clicked! " + index);
                                table1.currentIndex = index;
                                popup1.click(StandardButton.Save);
                            }
                        }
                    }
                  }
                Item {
                    Layout.fillWidth: true
                    height: closeButton.height
                    Button {
                        id: closeButton
                        anchors.centerIn: parent
                        text: "Close"
                        onClicked: {
                            popup1.click(StandardButton.Cancel);
                        }
                    }
                }
            }
        }
    }
}
