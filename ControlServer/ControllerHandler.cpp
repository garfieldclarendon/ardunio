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
        int classCode = urlQuery.queryItemValue("classCode").toInt();

        qDebug(QString("handleConfigUrl: controller %1").arg(serialNumber).toLatin1());
        Database db;
        if(classCode == ClassMulti)
            returnPayload = db.getMultiControllerConfig(serialNumber);
        qDebug(returnPayload.toLatin1());
    }
}

