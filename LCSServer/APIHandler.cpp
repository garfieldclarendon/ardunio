#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlError>

#include "GlobalDefs.h"
#include "APIHandler.h"
#include "DeviceManager.h"
#include "TurnoutHandler.h"
#include "RouteHandler.h"
#include "Database.h"
#include "WebServer.h"
#include "MessageBroadcaster.h"
#include "ControllerManager.h"
#include "EntityMetadata.h"
#include "APIEntity.h"

APIHandler::APIHandler(QObject *parent)
    : QObject(parent)
{

}

void APIHandler::handleClient(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload)
{
    QUrl url(path);

    if(url.path().contains("activate_turnout"))
    {
        handleActivateTurnout(socket, url, actionText, payload);
    }
    else if(url.path().contains("activate_route"))
    {
        handleActivateRoute(socket, url, actionText, payload);
    }
    else if(url.path().contains("panel_routes"))
    {
        handleGetPanelRouteList(socket, url);
    }
    else if(url.path().contains("panel_list"))
    {
        handleGetPanelList(socket, url);
    }
    else if(url.path().contains("device_list"))
    {
        handleGetDeviceList(socket, url);
    }
    else if(url.path().contains("device_property_list"))
    {
        handleGetDevicePropertyList(socket, url);
    }
    else if(url.path().contains("controller_list"))
    {
        handleGetControllerList(socket, url);
    }
    else if(url.path().contains("controller_module_list"))
    {
        handleGetControllerModuleList(socket, url);
    }
    else if(url.path().contains("send_module_config"))
    {
        handleSendModuleConfig(socket, url);
    }
    else if(url.path().contains("send_device_config"))
    {
        handleSendDeviceConfig(socket, url);
    }
    else if(url.path().contains("notification_list"))
    {
        handleResetNotificationList(socket, url);
    }
    else if(url.path().contains("lock_route"))
    {
        handleLockRoute(socket, url);
    }
    else if(url.path().contains("send_controller_firmware"))
    {
        handleControllerFirmwareUpdate(socket, url);
    }
    else if(url.path().contains("send_controller_reset_config"))
    {
        handleControllerResetConfig(socket, url);
    }
    else if(url.path().contains("send_controller_reset"))
    {
        handleControllerReset(socket, url);
    }
    else if(url.path().contains("entity/"))
    {
//        handleEntity(socket, url, actionText, payload);
        APIEntity entity;
        entity.handleClient(socket, path, actionText, payload);
    }
    else if(url.path().contains("signal_aspect_list"))
    {
        handleGetSignalAspectList(socket, url);
    }
    else if(url.path().contains("signal_condition_list"))
    {
        handleGetSignalConditionList(socket, url);
    }
    else if(url.path().contains("route_list"))
    {
        handleGetRouteList(socket, url);
    }
    else if(url.path().contains("route_entry_list"))
    {
        handleGetRouteEntryList(socket, url);
    }
    else if(url.path().contains("copy_device"))
    {
        handleCopyDevice(socket, url);
    }
    else
    {
        QString result = QString("HTTP/1.0 200 OK\r\n"
                                              "\r\n");

        qDebug("SEND OK");
        socket->write(result.toLatin1());
        socket->flush();
        socket->close();
    }
}

void APIHandler::handleActivateTurnout(QTcpSocket *socket, const QUrl &url, const QString &, const QString &)
{
    QString result = QString("HTTP/1.0 200 OK\r\n"
                                          "\r\n");

    qDebug("SEND OK");
    socket->write(result.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    TurnoutState newState = (TurnoutState)urlQuery.queryItemValue("turnoutState").toInt();

    if(newState == TrnUnknown)
    {
        TurnoutState currentState = (TurnoutState)DeviceManager::instance()->getDeviceStatus(deviceID);
        if(currentState == TrnNormal || currentState == TrnToDiverging)
            newState = TrnDiverging;
        else
            newState = TrnNormal;
    }
    if((newState == TrnNormal || newState == TrnDiverging))
    {
        TurnoutHandler *turnoutHandler = qobject_cast<TurnoutHandler *>(DeviceManager::instance()->getHandler(DeviceTurnout));
        if(turnoutHandler)
            turnoutHandler->activateTurnout(deviceID, newState);
    }
    else
    {
        // TODO:
        // Probably should return an error here
    }
}

void APIHandler::handleActivateRoute(QTcpSocket *socket, const QUrl &url, const QString &, const QString &)
{
    QString result = QString("HTTP/1.0 200 OK\r\n"
                                          "\r\n");

    qDebug("SEND OK");
    socket->write(result.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int routeID = urlQuery.queryItemValue("routeID").toInt();

    if(routeID > 0)
    {
        RouteHandler::instance()->activateRoute(routeID);
    }
    else
    {
        // TODO:
        // Probably should return an error here
    }
}

void APIHandler::handleGetPanelRouteList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int panelID = urlQuery.queryItemValue("panelID").toInt();

    qDebug(QString("handleGetPanelRouteList.  PanelID = %1").arg(panelID).toLatin1());
    Database db;

    QString sql = QString("SELECT ID as routeID, routeName, routeDescription FROM Route WHERE routeID IN (SELECT inputID FROM PanelInputEntry WHERE panelModuleID = %1) ORDER BY routeID").arg(panelID);
    QJsonArray jsonArray = db.fetchItems(sql);

    for(int x = 0; x < jsonArray.size(); x++)
    {
        QJsonObject obj = jsonArray[x].toObject();
        QString routeID = obj["routeID"].toString();
        bool isActive = RouteHandler::instance()->isRouteActive(routeID.toInt());
        bool isLocked = RouteHandler::instance()->isRouteLocked(routeID.toInt());
        bool canLock = RouteHandler::instance()->canRouteLock(routeID.toInt());
        obj["isActive"] = isActive;
        obj["isLocked"] = isLocked;
        obj["canLocked"] = canLock;
        jsonArray[x] = obj;
    }

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();

}

void APIHandler::handleGetRouteList(QTcpSocket *socket, const QUrl &)
{
    qDebug(QString("handleGetRouteList.").toLatin1());
    Database db;

    QString sql = QString("SELECT ID as routeID, routeName, routeDescription FROM Route ORDER BY routeName");
    QJsonArray jsonArray = db.fetchItems(sql);

    for(int x = 0; x < jsonArray.size(); x++)
    {
        QJsonObject obj = jsonArray[x].toObject();
        QString routeID = obj["routeID"].toString();
        bool isActive = RouteHandler::instance()->isRouteActive(routeID.toInt());
        bool isLocked = RouteHandler::instance()->isRouteLocked(routeID.toInt());
        bool canLock = RouteHandler::instance()->canRouteLock(routeID.toInt());
        obj["routeID"] = routeID;
        obj["isActive"] = isActive;
        obj["isLocked"] = isLocked;
        obj["canLock"] = canLock;
        jsonArray[x] = obj;
    }

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleGetRouteEntryList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int routeID = urlQuery.queryItemValue("routeID").toInt();

    qDebug(QString("handleGetRouteEntryList.  routeID = %1").arg(routeID).toLatin1());
    Database db;

    QString sql = QString("SELECT ID as routeEntryID, routeID, deviceID, turnoutState FROM RouteEntry WHERE routeID = %1").arg(routeID);
    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();}

void APIHandler::handleGetPanelList(QTcpSocket *socket, const QUrl &)
{
    qDebug(QString("handleGetPanelList.  SerialNumber = %1 address = %2").toLatin1());
    Database db;

    QString sql = QString("SELECT controllerModule.id as panelID, moduleName as panelName, controllerModule.address, serialNumber FROM controllerModule JOIN controller ON controller.id = controllerModule.controllerID WHERE deviceClass = 2 ORDER BY moduleName");

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleGetDeviceList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
    int moduleID = urlQuery.queryItemValue("moduleID").toInt();
    int controllerID = urlQuery.queryItemValue("controllerID").toInt();
    int port = -1;
    int classCode = urlQuery.queryItemValue("classCode").toInt();

    qDebug(QString("handleGetDeviceList.  SerialNumber = %1 address = %2").arg(serialNumber).arg(port).toLatin1());

    QJsonArray jsonArray = getDeviceList(serialNumber, controllerID, moduleID, classCode, -1);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->waitForBytesWritten();
//    socket->close();
}

void APIHandler::handleGetDevicePropertyList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    qDebug(QString("handleGetDevicePropertyList.  deviceID = %1").arg(deviceID).toLatin1());
    Database db;

    QString sql = QString("SELECT deviceID, id, key, value FROM deviceProperty WHERE deviceID = %1").arg(deviceID);

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->waitForBytesWritten();
    socket->close();
}

void APIHandler::handleGetControllerList(QTcpSocket *socket, const QUrl & url)
{
    qDebug(QString("handleGetModuleList.").toLatin1());
    QUrlQuery urlQuery(url);
    int controllerID = urlQuery.queryItemValue("controllerID").toInt();
    Database db;

    QString sql = QString("SELECT serialNumber, id as controllerID, controllerName, controllerDescription, controllerClass FROM controller");
    if(controllerID > 0)
        sql += QString(" WHERE id = %1").arg(controllerID);
    else
        sql += QString(" ORDER BY controllerName");

    QJsonArray jsonArray = db.fetchItems(sql);

    for(int x = 0; x < jsonArray.size(); x++)
    {
        QJsonObject obj = jsonArray[x].toObject();
        QString serialNumber = obj["serialNumber"].toString();
        QString version = "0.0.0";
        ControllerStatusEnum status = ControllerStatusUnknown;
        ControllerManager::instance()->getConnectedInfo(serialNumber.toInt(), version, status);
        obj["status"] = status;
        obj["version"] = version;
        obj["pingLength"] = -1;

        jsonArray[x] = obj;
    }


    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleGetControllerModuleList(QTcpSocket *socket, const QUrl &url)
{
    qDebug(QString("handleGetModuleList.").toLatin1());
    Database db;

    QUrlQuery urlQuery(url);
    int controllerID = urlQuery.queryItemValue("controllerID").toInt();
    int controllerModuleID = urlQuery.queryItemValue("controllerModuleID").toInt();
    QString sql = QString("SELECT id as controllerModuleID, controllerID, moduleName, moduleClass, address, disable FROM controllerModule");
    if(controllerID > 0)
        sql += QString(" WHERE controllerID = %1").arg(controllerID);
    else if(controllerModuleID > 0)
        sql += QString(" WHERE id = %1").arg(controllerModuleID);
    sql += QString(" ORDER BY address");

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleGetSignalAspectList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    qDebug(QString("handleGetSignalAspectList.").toLatin1());
    Database db;

    QString sql = QString("SELECT id as signalAspectID, deviceID, sortIndex, redMode, yellowMode, GreenMode FROM signalAspect");
    if(deviceID > 0)
        sql += QString(" WHERE deviceID = %1").arg(deviceID);
    sql += QString(" ORDER BY sortIndex");

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleGetSignalConditionList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int aspectID = urlQuery.queryItemValue("aspectID").toInt();
    qDebug(QString("handleGetSignalConditionList.").toLatin1());
    Database db;

    QString sql = QString("SELECT id as signalConditionID, signalAspectID, deviceID, conditionOperand, deviceState FROM signalCondition");
    if(aspectID > 0)
        sql += QString(" WHERE signalAspectID = %1").arg(aspectID);

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleSendModuleConfig(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
    int address = urlQuery.queryItemValue("address").toInt();
    qDebug(QString("handleGetPanelList.  SerialNumber = %1 address = %2").toLatin1());
    Database db;

    QString sql = QString("SELECT controllerModule.deviceClass, serialNumber FROM controllerModule JOIN controller ON controller.id = controllerModule.controllerID WHERE serialNumber = %1 AND address = %2").arg(serialNumber).arg(address);
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        int deviceClass = query.value("deviceClass").toInt();
        if(deviceClass == DeviceTurnout)
        {
            TurnoutHandler *turnoutHandler = qobject_cast<TurnoutHandler *>(DeviceManager::instance()->getHandler(DeviceTurnout));
            if(turnoutHandler)
            {
                turnoutHandler->sendConfig(serialNumber, address);
            }
        }
    }
}

void APIHandler::handleSendDeviceConfig(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    qDebug(QString("handleSendDeviceConfig.  DeviceID = %1").arg(deviceID).toLatin1());

    UDPMessage message;
    message.setMessageID(SYS_RESET_DEVICE_CONFIG);
    message.setID(deviceID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void APIHandler::handleControllerFirmwareUpdate(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendDownloadFirmware(serialNumber);
}

void APIHandler::handleControllerReset(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetCommand(serialNumber);
}

void APIHandler::handleControllerResetConfig(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetConfigCommand(serialNumber);
}

void APIHandler::handleResetNotificationList(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetNotificationListCommand(serialNumber);
}

void APIHandler::handleLockRoute(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int routeID = urlQuery.queryItemValue("routeID").toInt();
    bool lock= urlQuery.queryItemValue("lock").toInt() == 1;

    RouteHandler::instance()->lockRoute(routeID, lock);
}

void APIHandler::handleCopyDevice(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();

    // Get the original device
    QJsonArray deviceArray = getDeviceList(-1, -1, -1,-1, deviceID);

    QJsonObject obj = deviceArray[0].toObject();

    // Set its deviceID to -1 so that a new, auto-generated ID is created.
    obj["deviceID"] = -1;
    QJsonDocument doc;
    doc.setObject(obj);
    QByteArray data(doc.toJson());

    // Save the new device.
    APIEntity entity;
    QString newJson = entity.addEntity("device", data);

    doc = QJsonDocument::fromJson(newJson.toLatin1());
    obj = doc.object();
    int newDeviceID = obj["deviceID"].toVariant().toInt();
    // Copy the device properties from the original device to the newly created device.
    copyDeviceProperties(deviceID, newDeviceID);

    // Return the new device.
    QString header = WebServer::createHeader("200 OK", newJson.toLatin1().size());

    socket->write(header.toLatin1());
    socket->write(newJson.toLatin1());
    socket->flush();
    socket->close();
}

QJsonArray APIHandler::getDeviceList(long serialNumber, int controllerID, int moduleID, int classCode, int deviceID)
{
    Database db;

    QString sql = QString("SELECT device.id as deviceID, deviceName, deviceDescription, device.port, controllerModule.address, controllerModule.id as controllerModuleID, deviceClass, serialNumber, controller.id as controllerID, controllerModule.moduleClass FROM device LEFT OUTER JOIN controllerModule ON device.controllerModuleID = controllerModule.id LEFT OUTER JOIN controller ON controller.id = controllerModule.controllerID");

    QString where(" WHERE ");
    bool useAnd = false;
    if(serialNumber > 0)
    {
        useAnd = true;
        where += QString("serialNumber = %1 ").arg(serialNumber);
    }
    if(controllerID > 0)
    {
        if(useAnd)
            where += QString("AND controllerID = %1 ").arg(controllerID);
        else
            where += QString("controllerID = %1 ").arg(controllerID);

        useAnd = true;
    }
    if(moduleID > 0)
    {
        if(useAnd)
            where += QString("AND controllerModuleID = %1 ").arg(moduleID);
        else
            where += QString("controllerModuleID = %1 ").arg(moduleID);

        useAnd = true;
    }
    if(classCode > 0)
    {
        if(useAnd)
            where += QString("AND deviceClass = %1 ").arg(classCode);
        else
            where += QString("deviceClass = %1 ").arg(classCode);

        useAnd = true;
    }
    if(deviceID > 0)
    {
        useAnd = true;
        where = QString(" WHERE device.id = %1").arg(deviceID);
    }

    if(useAnd)
        sql += where;
    if(moduleID > 0 || controllerID > 0)
        sql += QString(" ORDER BY device.port");
    else
        sql += QString(" ORDER BY deviceName");

    QJsonArray jsonArray = db.fetchItems(sql);
    for(int x = 0; x < jsonArray.size(); x++)
    {
        QJsonObject obj = jsonArray[x].toObject();
        QString s = obj["deviceID"].toString();
        int deviceID = s.toInt();
        int deviceState = DeviceManager::instance()->getDeviceStatus(deviceID);
        obj["deviceState"] = deviceState;
        jsonArray[x] = obj;
    }

    return jsonArray;
}

void APIHandler::copyDeviceProperties(int fromID, int toID)
{
    QString sql = QString("INSERT INTO deviceProperty (deviceID, key, value) SELECT %1, key, value FROM deviceProperty WHERE deviceID = %2").arg(toID).arg(fromID);
    Database db;
    db.executeQuery(sql);
}
