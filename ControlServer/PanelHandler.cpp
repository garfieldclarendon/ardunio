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
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)), Qt::QueuedConnection);
//    connect(RouteHandler::instance(), SIGNAL(routeStatusChanged(int,bool)), this, SLOT(routeChanged(int, bool)));
}

void PanelHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("PanelHandler::deviceStatusChanged.  deviceID: %1  status: %2").arg(deviceID).arg(status).toLatin1());

    QString sql = QString("SELECT pinIndex, onValue, flashingValue, address, serialNumber FROM panelOutputEntry JOIN controllerModule ON panelOutputEntry.panelModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE itemID = %1 AND controllerModule.disable = 0 ORDER BY controller.serialNumber, controllerModule.address").arg(deviceID);
    Database db;
    QSqlQuery query1 = db.executeQuery(sql);

    if(query1.next())
    {
        int pinIndex;
        int address;
        int serialNumber = 0;
        int onValue;
        int flashingValue;
        QList<QJsonObject> jsons;
        QList<int> serialNumbers;
        QJsonArray jsonArray;

        int currentSerialNumber = query1.value("serialNumber").toInt();
        int currentaddress = query1.value("address").toInt();

        do
        {
            address = query1.value("address").toInt();
            serialNumber = query1.value("serialNumber").toInt();

            if(currentSerialNumber != serialNumber || currentaddress != address)
            {
                QJsonObject root;
                root["messageUri"] = "/controller/module";
                root["address"] = currentaddress;
                root["class"] = (int)ClassPanel;
                root["action"] = (int)NetActionUpdate;
                root["pins"] = jsonArray;

                jsons << root;
                serialNumbers << currentSerialNumber;
                currentSerialNumber = serialNumber;
                currentaddress = address;
                jsonArray = QJsonArray();
            }

            pinIndex = query1.value("pinIndex").toInt();
            onValue = query1.value("onValue").toInt();
            flashingValue = query1.value("flashingValue").toInt();
            QJsonObject obj;
            obj["pinIndex"] = pinIndex;
            if(onValue == status)
                obj["pinState"] = (int)PinOn;
            else if(flashingValue == status)
                obj["pinState"] = (int)PinFlashing;
            else
                obj["pinState"] = (int)PinOff;

    #ifdef Q_OS_WIN
            emit pinStateChanged(address, pinIndex, obj["pinState"].toInt());
    #endif
            jsonArray.append(obj);
        }
        while(query1.next());

        QJsonObject root;
        root["messageUri"] = "/controller/module";
        root["address"] = currentaddress;
        root["class"] = (int)ClassPanel;
        root["action"] = (int)NetActionUpdate;
        root["pins"] = jsonArray;

        jsons << root;
        serialNumbers << currentSerialNumber;

        for(int x = 0; x < jsons.count(); x++)
        {
            ControllerMessage message(serialNumbers.value(x), jsons.value(x));
            ControllerManager::instance()->sendMessage(message);
        }
    }
}

void PanelHandler::routeChanged(int routeID, bool isActive)
{
    qDebug(QString("PanelHandler::routeChanged.  routeID: %1  isActive: %2").arg(routeID).arg(isActive).toLatin1());

    int pinIndex;
    int address;
    int onValue;
    int pinRouteID;
    QString ipAddress;
    QStringList ips;
    QStringList urls;
    QList<int> serialNumbers;
    QList<QJsonObject> jsons;
    int currentSerialNumber = 0, currentaddress = 0;
    QMap<int, int> routeStatusMap;

    fillRouteStatusMap(routeStatusMap, routeID);

    QString sql = QString("SELECT pinIndex, onValue, flashingValue, address, serialNumber, routeID FROM panelOutputEntry JOIN panelRoute ON panelOutputEntry.id = panelRoute.panelOutputID JOIN controllerModule ON panelOutputEntry.panelModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE panelOutputEntry.id IN (SELECT panelOutputID FROM panelRoute WHERE routeID = %1) ORDER BY controller.id, controllerModule.address").arg(routeID);
    Database db;
    QSqlQuery query1 = db.executeQuery(sql);

    QJsonArray jsonArray;
    while(query1.next())
    {
        pinIndex = query1.value("pinIndex").toInt();
        address = query1.value("address").toInt();
        int serialNumber = query1.value("serialNumber").toInt();
        onValue = query1.value("onValue").toInt();
        pinRouteID = query1.value("routeID").toInt();

        if(currentSerialNumber == 0)
        {
            currentSerialNumber = serialNumber;
            currentaddress = address;
        }
        ipAddress = ControllerManager::instance()->getControllerIPAddress(serialNumber);
        if(ipAddress.length() > 0)
        {
            QJsonObject obj;
            obj["pinIndex"] = pinIndex;
            if(onValue == routeStatusMap[pinRouteID])
                obj["pinState"] = (int)PinOn;
            else
                obj["pinState"] = (int)PinOff;
#ifdef Q_OS_WIN
            emit pinStateChanged(address, pinIndex, obj["pinState"].toInt());
#endif
            jsonArray.append(obj);
            if(currentSerialNumber != serialNumber || currentaddress != address)
            {
                QJsonObject root;
                root["messageUri"] = "/controller/module";
                root["address"] = address;
                root["class"] = (int)ClassPanel;
                root["action"] = (int)NetActionUpdate;
                root["pins"] = jsonArray;

                QString uri = QString("/controller/module?address=%1").arg(address);
                ips << ipAddress;
                urls << uri;
                jsons << root;
                serialNumbers << currentSerialNumber;
            }
        }
    }
    if(ipAddress.length() > 0)
    {
        QJsonObject root;
        root["messageUri"] = "/controller/module";
        root["address"] = address;
        root["class"] = (int)ClassPanel;
        root["action"] = (int)NetActionUpdate;
        root["pins"] = jsonArray;

        QString uri = QString("/controller/module?address=%1").arg(address);
        ips << ipAddress;
        urls << uri;
        jsons << root;
    }
    for(int x = 0; x < urls.count(); x++)
    {
        ControllerMessage message(serialNumbers.value(x), jsons.value(x));
        ControllerManager::instance()->sendMessage(message);
        QJsonDocument doc;
        doc.setObject(jsons.value(x));
        qDebug(doc.toJson());
    }
}

void PanelHandler::newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json)
{
    if(uri == "/controller/module" && classCode == ClassPanel)
    {
        if(actionType == NetActionUpdate)
        {
            int pin;
            // For existing panle controllers.  This can be removed after those controllers are retired
            if(json.contains("buttonIndex"))
                pin = json["buttonIndex"].toInt();
            else
                pin = json["pin"].toInt();

            int routeID = getRouteID(serialNumber, address, pin);
            qDebug(QString("PanelHandler::newMessage:  Activate Route: %1").arg(routeID).toLatin1());

            if(routeID > 0)
            {
                RouteHandler::instance()->activateRoute(routeID);
            }
        }
        else if(actionType == NetActionGet && classCode == ClassPanel)
        {
            QString sql = QString("SELECT pinIndex, onValue, flashingValue, itemID FROM panelOutputEntry JOIN controllerModule ON panelOutputEntry.panelModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE serialNumber = %1 AND controllerModule.address = %2").arg(serialNumber).arg(address);
            Database db;
            QSqlQuery query1 = db.executeQuery(sql);

            QJsonObject obj;
            QJsonArray jsonArray;

            obj["address"] = address;
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
            ControllerMessage message(serialNumber, obj);
            ControllerManager::instance()->sendMessage(message);
        }
    }
}

int PanelHandler::getRouteID(int serialNumber, int address, int pin)
{
    qDebug(QString("GETROUTEID: Serial Number %1 Address: %2  Pin: %3").arg(serialNumber).arg(address).arg(pin).toLatin1());
    int routeID = 0;

    QString sql = QString("SELECT inputID as routeID FROM panelInputEntry JOIN controllerModule ON panelInputEntry.panelModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE controller.serialNumber = %1 AND controllerModule.address = %2 AND pinIndex = %3").arg(serialNumber).arg(address).arg(pin);
    Database db;
    QSqlQuery query1 = db.executeQuery(sql);
    while(query1.next())
    {
        routeID = query1.value("routeID").toInt();
    }

    return routeID;
}

void PanelHandler::fillRouteStatusMap(QMap<int, int> statusMap, int routeID)
{
    QList<int> routes;
    {
        QString sql = QString("SELECT routeID FROM panelOutputEntry JOIN panelRoute ON panelOutputEntry.id = panelRoute.panelOutputID WHERE panelOutputEntry.id IN (SELECT panelOutputID FROM panelRoute WHERE routeID = %1)").arg(routeID);
        Database db;
        QSqlQuery query1 = db.executeQuery(sql);
        while(query1.next())
        {
            int id = query1.value("routeID").toInt();
            if(routes.contains(id) == false)
                routes << id;
        }
    }
    for(int x = 0; x < routes.count(); x++)
    {
        statusMap[routes.value(x)] = RouteHandler::instance()->isRouteActive(routes.value(x));
    }
}
