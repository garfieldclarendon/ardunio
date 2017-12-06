#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

#include "ControllerHandler.h"
#include "Database.h"

ControllerHandler::ControllerHandler(QObject *parent)
    : QObject(parent)
{

}

void ControllerHandler::handleUrl(NetActionType actionType, const QUrl &url, const QString &payload, QString &returnPayload)
{
    Q_UNUSED(payload);

    if(actionType == NetActionGet)
    {
        QUrlQuery urlQuery(url);
        int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
        qDebug(QString("handleController: controller %1").arg(serialNumber).toLatin1());
        QString controllerName;
        int controllerID;
        Database db;
        db.getControllerIDAndName(serialNumber, controllerID, controllerName);
        QJsonDocument jsonDoc;
        QJsonObject obj;
        obj["controllerID"] = controllerID;
        obj["controllerName"] = controllerName;
        QString sql;
        sql = QString("SELECT device.id as deviceID, deviceName FROM  device JOIN controllerModule ON device.controllerModuleID = controllerModule.id  WHERE controllerID = %1").arg(controllerID);
        QJsonArray a = db.fetchItems(sql);
        obj["devices"] = a;
        jsonDoc.setObject(obj);
        returnPayload = jsonDoc.toJson();
        qDebug(returnPayload.toLatin1());
    }
}

void ControllerHandler::handleConfigUrl(NetActionType actionType, const QUrl &url, const QString &payload, QString &returnPayload)
{
    Q_UNUSED(payload);

    if(actionType == NetActionGet)
    {
        QUrlQuery urlQuery(url);
        int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

        qDebug(QString("handleConfigUrl: controller %1").arg(serialNumber).toLatin1());
        Database db;
        returnPayload = db.getMultiControllerConfig(serialNumber);
        qDebug(returnPayload.toLatin1());
        qDebug(QString("PAYLOAD SIZE: %1").arg(returnPayload.toLatin1().length()).toLatin1());
    }
}

void ControllerHandler::handleModuleConfigUrl(NetActionType actionType, const QUrl &url, const QString &payload, QString &returnPayload)
{
    Q_UNUSED(payload);

    if(actionType == NetActionGet)
    {
        QUrlQuery urlQuery(url);
        int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
        int address = urlQuery.queryItemValue("address").toInt();
        qDebug(QString("handleModuleConfigUrl: controller %1 address %2").arg(serialNumber).arg(address).toLatin1());
        Database db;
        returnPayload = db.getControllerModuleConfig(serialNumber, address);
        qDebug(returnPayload.toLatin1());
        qDebug(QString("PAYLOAD SIZE: %1").arg(returnPayload.toLatin1().length()).toLatin1());
    }
}

void ControllerHandler::handleDeviceConfigUrl(NetActionType actionType, const QUrl &url, const QString &payload, QString &returnPayload)
{
    Q_UNUSED(payload);

    if(actionType == NetActionGet)
    {
        QUrlQuery urlQuery(url);
        int deviceID = urlQuery.queryItemValue("deviceID").toInt();

        qDebug(QString("ControllerHandler::handleDeviceConfigUrl: deviceID %1").arg(deviceID).toLatin1());
        Database db;
        returnPayload = db.getDeviceConfig(deviceID);
        qDebug(returnPayload.toLatin1());
    }
}

void ControllerHandler::handleGetNotificationListUrl(NetActionType actionType, const QUrl &url, const QString &payload, QString &returnPayload)
{
    Q_UNUSED(payload);

    if(actionType == NetActionGet)
    {
        QUrlQuery urlQuery(url);
        int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

        qDebug(QString("ControllerHandler::handleGetNotificationListUrl: deviceID %1").arg(serialNumber).toLatin1());
        Database db;
        QJsonArray array = db.getNotificationList(serialNumber);
        QJsonDocument doc;
        doc.setArray(array);
        returnPayload = doc.toJson();
        qDebug(returnPayload.toLatin1());
    }
}
