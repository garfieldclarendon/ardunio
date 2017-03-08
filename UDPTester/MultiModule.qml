import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import Utils 1.0
import "./"

Item {
    id: mainItem
    property int currentControllerID: 0
    property int serialNumber: 0
    property alias currentControllerClass: moduleModel.controllerClass
    property bool allowNewModule: true
    property bool allowNewDevice: true

    ControllerModuleModel {
        id: moduleModel
        controllerID: currentControllerID
        onAllowNewModuleChanged: {
            mainItem.allowNewModule = moduleModel.allowNewModule;
            console.debug("ControllerModuleModel::onAllowNewModuleChanged: " + mainItem.allowNewModule);
        }
    }

    function addNewModule(moduleName, moduleIndex, moduleClass)
    {
        if(moduleIndex === -1)
            moduleIndex = moduleModel.rowCount();
        moduleModel.addNew(moduleName, moduleIndex, moduleClass);
    }

    function addNewDevice(deviceName, moduleID, moduleIndex)
    {

    }

    ListView {
        anchors.fill: parent
        anchors.margins: ui.applyRatio(15)
        id: listView
        model: moduleModel

        delegate: switch (moduleModel.getModuleClass(listView.currentIndex))
                  {
            case 1: return turnoutDelegate;
            //....
            default: return dummyDelegate
        }
        spacing: ui.applyRatio(2)
        focus: true
        clip: true
    }
    Component {
        id: turnoutDelegate

        TurnoutController {
            id: turnoutController
            width: parent.width
            height: 300
            currentControllerModuleID: id
        }
    }

    Component {
        id: dummyDelegate
        Item {  }
      }
}

