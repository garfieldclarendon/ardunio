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
        handleEntity(socket, url, actionText, payload);
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
    socket->close();}

void APIHandler::handleGetDeviceList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
    int moduleID = urlQuery.queryItemValue("moduleID").toInt();
    int controllerID = urlQuery.queryItemValue("controllerID").toInt();
    int port = -1;
    int classCode = urlQuery.queryItemValue("classCode").toInt();
    if(urlQuery.hasQueryItem("address"))
        port = urlQuery.queryItemValue("address").toInt();

    qDebug(QString("handleGetDeviceList.  SerialNumber = %1 address = %2").arg(serialNumber).arg(port).toLatin1());
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
    if(port > -1)
    {
        if(useAnd)
            where += QString("AND port = %1 ").arg(port);
        else
            where += QString("port = %1 ").arg(port);

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

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
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
        int version = 0;
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
    message.setSerialNumber(deviceID);

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

void APIHandler::handleEntity(QTcpSocket *socket, const QUrl &url, const QString &actionText, const QString &jsonText)
{
    QStringList parts = url.path().split('/', QString::SkipEmptyParts);
    QString entityName = parts.value(parts.count() - 1);
    qDebug(QString("HANDLEENTITY:  PATH: %1").arg(url.path()).toLatin1());
    qDebug(QString("HANDLEENTITY:  ENTITY NAME: %1").arg(entityName).toLatin1());

    QByteArray data(jsonText.toLatin1());

    if(actionText == "PUT")
        data = saveEntity(entityName, jsonText).toLatin1();
    else if(actionText == "POST")
        data = addEntity(entityName, jsonText).toLatin1();
    else if(actionText == "DELETE")
        deleteEntity(entityName, jsonText);
    else
        data = fetchEntity(entityName).toLatin1();

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();

}

QString APIHandler::fetchEntity(const QString &name)
{
    Database db;

    QString sql = QString("SELECT * FROM %1").arg(EntityMetadata::instance()->getTableName(name));
    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    return doc.toJson();
}

QString APIHandler::saveEntity(const QString &name, const QString &jsonText)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toLatin1());
    QJsonObject obj = doc.object();
    Database db;
    QSqlTableModel model(this, db.getDatabase());
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);

    QString keyField = EntityMetadata::instance()->getKeyField(name);
    QString tableKey = EntityMetadata::instance()->getTableField(name);
    QString tableName = EntityMetadata::instance()->getTableName(name);
    QStringList skip;
    skip << "keyField" << "tableKeyField" << "tableName" << keyField;

    int id = obj[keyField].toString().toInt();

    qDebug(QString("handleSaveEntity.  ID = %1").arg(id).toLatin1());

    QString filter = QString("%1 = %2").arg(tableKey).arg(id);
    model.setTable(tableName);
    model.setFilter(filter);
    if(model.select() == false)
    {
        QString error = model.lastError().text();
        obj["dbError"] = error;
    }

    if(model.rowCount() > 0)
    {
        QSqlRecord r = model.record(0);
        foreach (QString key, obj.keys())
        {
            if(skip.contains(key) == false)
            {
                QJsonValue val = obj[key];
                QVariant v = val.toVariant();
                r.setValue(key, v);
            }
        }
        model.setRecord(0, r);
        if(model.submitAll() == false)
        {
            QString error = model.lastError().text();
            obj["dbError"] = error;
        }
    }
    else
    {
        obj["dbError"] = QString("Failed to find record in table '%1' with the key value '%2'").arg(tableName).arg(id);
    }
    QJsonDocument retDoc;
    retDoc.setObject(obj);

    return retDoc.toJson();
}

QString APIHandler::addEntity(const QString &name, const QString &jsonText)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toLatin1());
    QJsonObject obj = doc.object();
    Database db;
    QSqlTableModel model(this, db.getDatabase());
    model.setEditStrategy(QSqlTableModel::OnManualSubmit);

    QString keyField = EntityMetadata::instance()->getKeyField(name);
    QString tableKey = EntityMetadata::instance()->getTableField(name);
    QString tableName = EntityMetadata::instance()->getTableName(name);
    QStringList skip;
    skip << "keyField" << "tableKeyField" << "tableName" << keyField;

    int id = obj[keyField].toString().toInt();

    qDebug(QString("addEntity.  ID = %1").arg(id).toLatin1());

    QString filter = QString("%1 = %2").arg(tableKey).arg(id);
    model.setTable(tableName);
    model.setFilter(filter);
    model.select();
    model.insertRow(model.rowCount());

    if(model.rowCount() > 0)
    {
        QSqlRecord r = model.record(0);
        foreach (QString key, obj.keys())
        {
            if(skip.contains(key) == false)
            {
                QJsonValue val = obj[key];
                QVariant v = val.toVariant();
                r.setValue(key, v);
                r.setGenerated(key, true);
            }
        }
        r.setGenerated(keyField, false);
        model.setRecord(0, r);
        if(model.submitAll() == false)
        {
            QString error = model.lastError().text();
            obj["dbError"] = error;
        }
        else
        {
            QSqlQuery q = db.executeQuery("SELECT last_insert_rowid()");

            if(q.next())
            {
                 int id = q.value(0).toInt();
                qDebug(QString("addEntity.  NEW ID = %1").arg(id).toLatin1());
                if(id > 0)
                    obj[keyField] = id;
            }
        }
    }
    else
    {
        obj["dbError"] = QString("Failed to find record in table '%1' with the key value '%2'").arg(tableName).arg(tableKey);
    }
    QJsonDocument retDoc;
    retDoc.setObject(obj);

    return retDoc.toJson();
}

void APIHandler::deleteEntity(const QString &name, const QString &jsonText)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonText.toLatin1());
    QJsonObject obj = doc.object();
    QString keyField = EntityMetadata::instance()->getKeyField(name);
    QString tableKey = EntityMetadata::instance()->getTableField(name);
    QString tableName = EntityMetadata::instance()->getTableName(name);
    int id = obj[keyField].toString().toInt();

    qDebug(QString("handleSaveDevice.  id = %1").arg(id).toLatin1());
    Database db;

    QString sql = QString("DELETE FROM %1 WHERE %2 = %3").arg(tableName).arg(tableKey).arg(id);
    qDebug(QString("handleSaveDevice.  %1").arg(sql).toLatin1());

    db.executeQuery(sql);
}

