#include <QUrl>
#include <QUrlQuery>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlQuery>

#include "APIDevice.h"
#include "MessageBroadcaster.h"
#include "WebServer.h"
#include "Database.h"
#include "DeviceManager.h"
#include "APIEntity.h"
#include "NotificationServer.h"
#include "DeviceManager.h"

APIDevice::APIDevice(QObject *parent) : QObject(parent)
{
    connect(this, &APIDevice::sendNotificationMessage, NotificationServer::instance(), &NotificationServer::sendNotificationMessage);
    connect(DeviceManager::instance(), &DeviceManager::deviceStatusChanged, this, &APIDevice::onDeviceStatusChanged);

    WebServer *webServer = WebServer::instance();

    UrlHandler *handler;

    handler = webServer->createUrlHandler("/api/device_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetDeviceList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/device_property_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetDevicePropertyList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/module_device_port_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetModuleDevicePortList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/send_device_config");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleSendDeviceConfig(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/create_device");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleCreateDevice(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/lock_device");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleLockDevice(APIRequest,APIResponse*)), Qt::DirectConnection);
}

void APIDevice::handleGetDeviceList(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
    int moduleID = urlQuery.queryItemValue("moduleID").toInt();
    int controllerID = urlQuery.queryItemValue("controllerID").toInt();
    int port = -1;
    int classCode = urlQuery.queryItemValue("classCode").toInt();
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();

    qDebug(QString("handleGetDeviceList.  SerialNumber = %1 address = %2").arg(serialNumber).arg(port).toLatin1());

    QJsonArray jsonArray = getDeviceList(serialNumber, controllerID, moduleID, classCode, deviceID);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APIDevice::handleGetDevicePropertyList(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    qDebug(QString("handleGetDevicePropertyList.  deviceID = %1").arg(deviceID).toLatin1());
    Database db;

    QString sql = QString("SELECT deviceID, id, key, value FROM deviceProperty WHERE deviceID = %1").arg(deviceID);

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APIDevice::handleGetModuleDevicePortList(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    int moduleID = urlQuery.queryItemValue("moduleID").toInt();
    qDebug(QString("handleGetDevicePropertyList.  deviceID = %1").arg(deviceID).toLatin1());
    Database db;

    QString sql;
    sql = QString("SELECT moduleDevicePort.id, deviceID, controllerModuleID, moduleName, moduleClass, deviceName, labelName, deviceClass, port  FROM moduleDevicePort LEFT OUTER JOIN controllerModule ON moduleDevicePort.controllerModuleID = controllerModule.id LEFT OUTER JOIN device ON moduleDevicePort.deviceID = device.id");

    if(moduleID > 0)
        sql += QString(" WHERE moduleDevicePort.controllerModuleID = %1 ORDER BY port").arg(moduleID);
    else if(deviceID > 0)
        sql += QString(" WHERE moduleDevicePort.deviceID = %1").arg(deviceID);

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APIDevice::handleSendDeviceConfig(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    qDebug(QString("handleSendDeviceConfig.  DeviceID = %1").arg(deviceID).toLatin1());

    UDPMessage message;
    message.setMessageID(SYS_RESET_DEVICE_CONFIG);
    message.setID(deviceID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void APIDevice::handleCreateDevice(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
    DeviceClassEnum deviceClass = (DeviceClassEnum)urlQuery.queryItemValue("deviceClass").toInt();
    if(deviceClass != DeviceUnknown)
    {
        Database db;
        QJsonObject obj;
        obj = db.createNewDevice(deviceClass);
        QJsonDocument doc;
        doc.setObject(obj);
        response->setPayload(doc.toJson());
    }
}

void APIDevice::handleLockDevice(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    bool lock = urlQuery.queryItemValue("deviceID").toInt() == 1;
    Database db;
    DeviceClassEnum deviceClass = db.getDeviceClass(deviceID);

    if(deviceClass == DeviceTurnout)
    {
        lockTurnout(deviceID, lock);
    }
    else if(deviceClass == DeviceSignal)
    {
        PinStateEnum redMode = (PinStateEnum)urlQuery.queryItemValue("redMode").toInt();
        PinStateEnum greenMode = (PinStateEnum)urlQuery.queryItemValue("greenMode").toInt();
        PinStateEnum yellowMode = (PinStateEnum)urlQuery.queryItemValue("yellowMode").toInt();

        lockSignal(deviceID, lock, redMode, greenMode, yellowMode);
    }
}

void APIDevice::onDeviceStatusChanged(int deviceID, int status, bool locked)
{
    createAndSendNotificationMessage(deviceID, status, locked);
}

void APIDevice::lockTurnout(int deviceID, bool lock)
{
    UDPMessage message;
    message.setMessageID(SYS_LOCK_DEVICE);
    message.setID(deviceID);
    message.setField(0, lock);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void APIDevice::lockSignal(int deviceID, bool lock, PinStateEnum redMode, PinStateEnum greenMode, PinStateEnum yellowMode)
{
    UDPMessage message;
    message.setMessageID(SYS_LOCK_DEVICE);
    message.setID(deviceID);
    message.setField(0, lock);

    // At least one pin setting must be set to either on or flashing.  If not, assume the parameters were not supplied.
    if(redMode != PinOff || greenMode != PinOff || yellowMode != PinOff)
    {
        message.setField(1, 1);
        message.setField(2, redMode);
        message.setField(3, greenMode);
        message.setField(4, yellowMode);
    }

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

QJsonArray APIDevice::getDeviceList(long serialNumber, int controllerID, int moduleID, int classCode, int deviceID)
{
    Database db;

    QString sql = QString("SELECT device.id as deviceID, deviceName, deviceDescription, deviceClass FROM device");

    QString where(" WHERE ");
    bool useAnd = false;
    if(serialNumber > 0)
    {
        useAnd = true;
        where += QString(" deviceID IN (select deviceID FROM moduleDevicePort JOIN controllerModule ON moduleDevicePort.controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE serialNumber = %1)").arg(serialNumber);
    }
    if(controllerID > 0)
    {
        if(useAnd)
            where += QString(" AND");
        where += QString(" deviceID IN (select deviceID FROM moduleDevicePort JOIN controllerModule ON moduleDevicePort.controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE controllerID = %1)").arg(controllerID);

        useAnd = true;
    }
    if(moduleID > 0)
    {
        if(useAnd)
            where += QString(" AND");
        where += QString(" deviceID IN (select deviceID FROM moduleDevicePort JOIN controllerModule ON moduleDevicePort.controllerModuleID = controllerModule.id WHERE controllerModuleID = %1)").arg(moduleID);

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

void APIDevice::createAndSendNotificationMessage(int deviceID, int newState, bool locked)
{
    QString uri("/api/notification/device");
    QJsonObject obj;
    obj["deviceID"] = QString("%1").arg(deviceID);
    obj["deviceState"] = QString("%1").arg(newState);
    obj["locked"] = QString("%1").arg(locked);

    emit sendNotificationMessage(uri, obj);
}
