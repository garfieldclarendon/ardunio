import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.2
import Utils 1.0


Item {
    id: mainItem
    anchors.margins: ui.margin

    ControllerModel {
        id: controllerModel
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: ui.margin
        StackView {
            id: stackView
            clip:true
            Layout.preferredWidth: mainItem.width * .55
            Layout.fillHeight: true

            Component {
                id: list
                ControllerList {
                    id: controllerList
                    model: controllerModel
                    onEditClicked: {
                        stackView.push({item:"qrc:/EditController.qml", properties:{model:model,index:index}})
                        stackView.currentItem.editFinished.connect(editFinished);
                    }

                    function editFinished(save)
                    {
                        stackView.pop();
                        if(save)
                            model.save();
                    }
                    onControllerIDChanged: {
                        console.debug();
                        if(currentControllerClass === 1)
                        {
                            detailLoader.setSource("TurnoutController.qml",
                                                         { "currentControllerID": controllerID });
                        }
                        else if(currentControllerClass === 2)
                        {
                            detailLoader.setSource("PanelController.qml",
                                                         { "currentControllerID": controllerID });
                        }
                        else
                        {
                            detailLoader.source = "";
                        }
                    }
                }
            }

            Component.onCompleted: {
                stackView.push(list);
            }
        }

        Loader {
            id: detailLoader
            anchors.margins: ui.margin
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

}

