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
    handler = webServer->createUrlHandler("/api/send_device_config");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleSendDeviceConfig(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/copy_device");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleCopyDevice(APIRequest,APIResponse*)), Qt::DirectConnection);
}

void APIDevice::handleGetDeviceList(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
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

void APIDevice::handleCopyDevice(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
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
    response->setPayload(doc.toJson());
}

void APIDevice::onDeviceStatusChanged(int deviceID, int status)
{
    createAndSendNotificationMessage(deviceID, status);
}

QJsonArray APIDevice::getDeviceList(long serialNumber, int controllerID, int moduleID, int classCode, int deviceID)
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

void APIDevice::copyDeviceProperties(int fromID, int toID)
{
    QString sql = QString("INSERT INTO deviceProperty (deviceID, key, value) SELECT %1, key, value FROM deviceProperty WHERE deviceID = %2").arg(toID).arg(fromID);
    Database db;
    db.executeQuery(sql);
}

void APIDevice::createAndSendNotificationMessage(int deviceID, int newState)
{
    QString uri("/api/notification/device");
    QJsonObject obj;
    obj["deviceID"] = QString("%1").arg(deviceID);
    obj["deviceState"] = QString("%1").arg(newState);

    emit sendNotificationMessage(uri, obj);
}
