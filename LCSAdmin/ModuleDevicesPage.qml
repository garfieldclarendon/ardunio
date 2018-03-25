import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2
import Utils 1.0

Item {
    property string pathName: "AssignedModules"
    property string title: "Assigned Modules"
    property alias moduleID: moduleDevices.moduleID

    ModuleDevices {
        id: moduleDevices
        anchors.fill: parent
    }
}

