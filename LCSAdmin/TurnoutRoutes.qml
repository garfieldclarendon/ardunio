import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Utils 1.0

Rectangle {
    id: turnoutRouteGroup
    implicitHeight: expanded ? (textMetrics.itemHeight * (routeManager.rowCount + 1)) + 15 : textMetrics.itemHeight
    border.color: "lightgrey"
    border.width: 1
    property int deviceID: -1
    property int deviceClass: 0
    property bool expanded: true
    property alias model: routeManager.model
    property alias currentIndex: routeManager.currentIndex

    signal editClicked(int index, var entity);
    signal addClicked();

    TextMetrics {
        id: textMetrics
        property int itemHeight: Math.max(addButton.height, textMetrics.height + 35)
        text: "CLASS>MULTI-CONTROLLER"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            expanded = !expanded;
        }
    }

    Text {
        id: titleText
        text: routeManager.rowCount > 0 ? "Assigned Routes (" + routeManager.rowCount + ")" : "Assigned Routes"
        height: Math.max(addButton.height, implicitHeight)
        font.pixelSize: textMetrics.height + (textMetrics.height * 0.5)
        font.bold: true
        horizontalAlignment: Qt.AlignHCenter
        anchors.topMargin: 5
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
    ToolButton {
        id: addButton
        visible: false
        anchors.right: parent.right
        anchors.top: parent.top

        contentItem: Image {
            source: "Images/add.png"
            sourceSize.width: 18
            sourceSize.height: 18
        }
        onClicked: {
            var entity = routeManager.model.getEntity(-1);
            routeManager.model.setEntity(-1, entity);
            routeManager.currentIndex = routeManager.rowCount - 1
        }
    }

    Item {
        id: containerItem
        anchors.left: parent.left
        anchors.top: titleText.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        opacity: expanded ? 1 : 0

        ManageRoutes {
            id: routeManager
            deviceID: turnoutRouteGroup.deviceID
            anchors.fill: parent
            onAddClicked: {
                turnoutRouteGroup.addClicked();
            }
            onEditClicked: {
                turnoutRouteGroup.editClicked(index, entity);
            }
        }

        function saveData()
        {
            routeManager.model.save();
        }
    }
}
