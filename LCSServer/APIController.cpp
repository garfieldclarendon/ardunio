#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDir>

#include "APIController.h"
#include "WebServer.h"
#include "Database.h"
#include "ControllerManager.h"
#include "MessageBroadcaster.h"
#include "NotificationServer.h"

APIController::APIController(QObject *parent) : QObject(parent)
{
    connect(this, &APIController::sendNotificationMessage, NotificationServer::instance(), &NotificationServer::sendNotificationMessage);
    connect(ControllerManager::instance(), SIGNAL(controllerStatusChanged(long,ControllerStatusEnum,QString)), this, SLOT(onControllerStatusChanged(long,ControllerStatusEnum,QString)));

    WebServer *webServer = WebServer::instance();

    UrlHandler *handler;

    handler = webServer->createUrlHandler("/controller/config");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleConfigUrl(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/controller/module/config");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleModuleConfigUrl(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/controller/device/config");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleDeviceConfigUrl(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/controller/notification_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetNotificationListUrl(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/controller/firmware");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleDownloadFirmwareUrl(APIRequest,APIResponse*)), Qt::DirectConnection);

    handler = webServer->createUrlHandler("/api/controller_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetControllerList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/controller_module_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetControllerModuleList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/send_controller_reset");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleControllerReset(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/send_controller_reset_config");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleControllerResetConfig(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/send_controller_reset_notification_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleResetNotificationList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/send_controller_firmware");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleSendFirmware(APIRequest,APIResponse*)), Qt::DirectConnection);
}

void APIController::onControllerStatusChanged(long serialNumber, ControllerStatusEnum newStatus, const QString &version)
{
    createAndSendNotificationMessage(serialNumber, newStatus, version);
}

void APIController::handleConfigUrl(const APIRequest &request, APIResponse *response)
{
    if(request.getNetActionType() == NetActionGet)
    {
        QUrlQuery urlQuery(request.getUrl());
        int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

        qDebug(QString("handleConfigUrl: controller %1").arg(serialNumber).toLatin1());
        Database db;
        response->setPayload(db.getMultiControllerConfig(serialNumber));
        qDebug(response->getPayload());
        qDebug(QString("PAYLOAD SIZE: %1").arg(response->getPayload().size()).toLatin1());
    }
}

void APIController::handleModuleConfigUrl(const APIRequest &request, APIResponse *response)
{
    if(request.getNetActionType() == NetActionGet)
    {
        QUrlQuery urlQuery(request.getUrl());
        int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
        int address = urlQuery.queryItemValue("address").toInt();
        qDebug(QString("handleModuleConfigUrl: controller %1 address %2").arg(serialNumber).arg(address).toLatin1());
        Database db;
        response->setPayload(db.getControllerModuleConfig(serialNumber, address));
        qDebug(response->getPayload());
        qDebug(QString("PAYLOAD SIZE: %1").arg(response->getPayload().size()).toLatin1());
    }
}

void APIController::handleDeviceConfigUrl(const APIRequest &request, APIResponse *response)
{
    if(request.getNetActionType() == NetActionGet)
    {
        QUrlQuery urlQuery(request.getUrl());
        QString payload;
        if(urlQuery.hasQueryItem("aspectID"))
        {
            int aspectID = urlQuery.queryItemValue("aspectID").toInt();
            Database db;
            payload = db.getSignalAspectConfig(aspectID);
        }
        else
        {
            int deviceID = urlQuery.queryItemValue("deviceID").toInt();
            qDebug(QString("APIController::handleDeviceConfigUrl: deviceID %1").arg(deviceID).toLatin1());
            Database db;
            payload = db.getDeviceConfig(deviceID);
        }
        response->setPayload(payload.toLatin1());
        qDebug(payload.toLatin1());
    }
}

void APIController::handleGetNotificationListUrl(const APIRequest &request, APIResponse *response)
{
    if(request.getNetActionType() == NetActionGet)
    {
        QUrlQuery urlQuery(request.getUrl());
        int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

        qDebug(QString("APIController::handleGetNotificationListUrl: deviceID %1").arg(serialNumber).toLatin1());
        Database db;
        QJsonArray array = db.getNotificationList(serialNumber);
        QJsonDocument doc;
        doc.setArray(array);
        response->setPayload(doc.toJson());
        qDebug(response->getPayload());
    }
}

void APIController::handleDownloadFirmwareUrl(const APIRequest &, APIResponse *response)
{
    qDebug("DOWNLOAD FIRMAWARE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

    QString fileName;
    QString path = QCoreApplication::applicationDirPath();
    QDir::setCurrent(path);
    path += "/Builds/";
    fileName = path + "LCSController.ino.bin";

    qDebug(fileName.toLatin1());
    // PROCESS //
    QByteArray fileData;
    if(fileName.length() > 0)
        fileData = getFile(fileName);

    response->setPayload(fileData);
    response->setContenetType("application/octet-stream");
}

void APIController::handleGetControllerList(const APIRequest &request, APIResponse *response)
{
    qDebug(QString("handleGetModuleList.").toLatin1());
    QUrlQuery urlQuery(request.getUrl());
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

        jsonArray[x] = obj;
    }


    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APIController::handleGetControllerModuleList(const APIRequest &request, APIResponse *response)
{
    qDebug(QString("handleGetModuleList.").toLatin1());
    Database db;

    QUrlQuery urlQuery(request.getUrl());
    int controllerID = urlQuery.queryItemValue("controllerID").toInt();
    int controllerModuleID = urlQuery.queryItemValue("controllerModuleID").toInt();
    int moduleClass = urlQuery.queryItemValue("moduleClass").toInt();

    QString sql = QString("SELECT id as controllerModuleID, controllerID, moduleName, moduleClass, address, disable FROM controllerModule");
    if(controllerID > 0)
        sql += QString(" WHERE controllerID = %1").arg(controllerID);
    else if(controllerModuleID > 0)
        sql += QString(" WHERE id = %1").arg(controllerModuleID);
    else if(moduleClass > 0)
        sql += QString(" WHERE moduleClass = %1").arg(moduleClass);
    if(controllerID > 0)
        sql += QString(" ORDER BY address");
    else
        sql += QString(" ORDER BY moduleName");

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APIController::handleControllerReset(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetCommand(serialNumber);
}

void APIController::handleControllerResetConfig(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetConfigCommand(serialNumber);
}

void APIController::handleResetNotificationList(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetNotificationListCommand(serialNumber);
}

void APIController::handleSendFirmware(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendDownloadFirmware(serialNumber);
}

QByteArray APIController::getFile(const QString &fileName)
{
    QByteArray fileData;
    QFile file(fileName);

    if(file.open(QIODevice::ReadOnly))
    {
        fileData = file.readAll();
        file.close();
    }

    return fileData;
}

void APIController::createAndSendNotificationMessage(long serialNumber, ControllerStatusEnum status, const QString &version)
{
    QString url("/api/notification/controller");
    QJsonObject obj;
    obj["serialNumber"] = QString("%1").arg(serialNumber);
    obj["status"] = QString("%1").arg(status);
    obj["version"] = version;

    emit sendNotificationMessage(url, obj);
}
