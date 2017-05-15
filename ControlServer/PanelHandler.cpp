#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QVariant>

#include "PanelHandler.h"
#include "Database.h"
#include "RouteHandler.h"
#include "ControllerManager.h"
#include "DeviceManager.h"
#include "WebServer.h"

PanelHandler::PanelHandler(QObject *parent)
    : DeviceHandler(ClassPanel, parent)
{
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)));
}

void PanelHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("PanelHandler::deviceStatusChanged.  deviceID: %1  status: %2").arg(deviceID).arg(status).toLatin1());

    QString sql = QString("SELECT pinIndex, onValue, flashingValue, moduleIndex, serialNumber FROM panelOutputEntry JOIN controllerModule ON panelOutputEntry.panelModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE itemID = %1 ORDER BY controller.id, controllerModule.moduleIndex").arg(deviceID);
    Database db;
    QSqlQuery query1 = db.executeQuery(sql);
    int pinIndex;
    int moduleIndex;
    int serialNumber;
    int onValue;
    int flashingValue;
    QString ipAddress;
    QStringList ips;
    QStringList urls;
    QList<QJsonObject> jsons;
    int currentSerialNumber = 0, currentModuleIndex = 0;

    QJsonArray jsonArray;
    while(query1.next())
    {
        pinIndex = query1.value("pinIndex").toInt();
        moduleIndex = query1.value("moduleIndex").toInt();
        serialNumber = query1.value("serialNumber").toInt();
        onValue = query1.value("onValue").toInt();
        flashingValue = query1.value("flashingValue").toInt();
        if(currentSerialNumber == 0)
        {
            currentSerialNumber = serialNumber;
            currentModuleIndex = moduleIndex;
        }
        ipAddress = ControllerManager::instance()->getControllerIPAddress(serialNumber);
        if(ipAddress.length() > 0)
        {
            QJsonObject obj;
            obj["pinIndex"] = pinIndex;
            if(onValue == status)
                obj["pinState"] = (int)PinOn;
            else if(flashingValue == status)
                obj["pinState"] = (int)PinFlashing;
            else
                obj["pinState"] = (int)PinOff;
            jsonArray.append(obj);
            if(currentSerialNumber != serialNumber || currentModuleIndex != moduleIndex)
            {
                QJsonObject root;
                root["messageUri"] = "/controller/module";
                root["moduleIndex"] = moduleIndex;
                root["class"] = (int)ClassPanel;
                root["action"] = (int)NetActionUpdate;
                root["pins"] = jsonArray;

                QString uri = QString("/controller/module?moduleIndex=%1").arg(moduleIndex);
                ips << ipAddress;
                urls << uri;
                jsons << root;
            }
        }
    }
    if(ipAddress.length() > 0)
    {
        QJsonObject root;
        root["messageUri"] = "/controller/module";
        root["moduleIndex"] = moduleIndex;
        root["class"] = (int)ClassPanel;
        root["action"] = (int)NetActionUpdate;
        root["pins"] = jsonArray;

        QString uri = QString("/controller/module?moduleIndex=%1").arg(moduleIndex);
        ips << ipAddress;
        urls << uri;
        jsons << root;
    }
    for(int x = 0; x < urls.count(); x++)
    {
        ControllerManager::instance()->sendMessage(serialNumber, jsons.value(x));
    }
}

void PanelHandler::newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json)
{
    if(uri == "/controller/module" && classCode == ClassPanel)
    {
        if(actionType == NetActionUpdate)
        {
            int buttonIndex = json["buttonIndex"].toInt();

            int routeID = getRouteID(serialNumber, moduleIndex, buttonIndex);
            qDebug(QString("PanelHandler::newMessage:  Activate Route: %1").arg(routeID).toLatin1());

            if(routeID > 0)
            {
                RouteHandler::instance()->activateRoute(routeID);
            }
        }
        else if(actionType == NetActionGet && classCode == ClassPanel)
        {
            QString sql = QString("SELECT pinIndex, onValue, flashingValue, itemID FROM panelOutputEntry JOIN controllerModule ON panelOutputEntry.panelModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE serialNumber = %1 AND controllerModule.moduleIndex = %2").arg(serialNumber).arg(moduleIndex);
            Database db;
            QSqlQuery query1 = db.executeQuery(sql);

            QJsonObject obj;
            QJsonArray jsonArray;

            obj["moduleIndex"] = moduleIndex;
            obj["messageUri"] = "/controller/module";
            obj["action"] = NetActionUpdate;
            while(query1.next())
            {
                int itemID = query1.value("itemID").toInt();
                int status = DeviceManager::instance()->getDeviceStatus(itemID);
                int pinIndex = query1.value("pinIndex").toInt();
                int onValue = query1.value("onValue").toInt();
                int flashingValue = query1.value("flashingValue").toInt();

                QJsonObject obj;
                obj["pinIndex"] = pinIndex;
                if(onValue == status)
                    obj["pinState"] = (int)PinOn;
                else if(flashingValue == status)
                    obj["pinState"] = (int)PinFlashing;
                else
                    obj["pinState"] = (int)PinOff;
                jsonArray.append(obj);

            }
            obj["pins"] = jsonArray;
            ControllerManager::instance()->sendMessage(serialNumber, obj);
        }
    }
}

int PanelHandler::getRouteID(int serialNumber, int moduleIndex, int buttonIndex)
{
    int routeID = 0;

    QString sql = QString("SELECT inputID as routeID FROM panelInputEntry JOIN controllerModule ON panelInputEntry.panelModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE controller.serialNumber = %1 AND controllerModule.moduleIndex = %2 AND pinIndex = %3").arg(serialNumber).arg(moduleIndex).arg(buttonIndex);
    Database db;
    QSqlQuery query1 = db.executeQuery(sql);
    while(query1.next())
    {
        routeID = query1.value("routeID").toInt();
    }

    return routeID;
}
