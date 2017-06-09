#include <QList>
#include <QSqlQuery>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>

#include "SemaphoreHandler.h"
#include "Database.h"
#include "DeviceManager.h"
#include "ControllerManager.h"
#include "WebServer.h"

SemaphoreHandler::SemaphoreHandler(QObject *parent)
    : DeviceHandler(ClassSemaphore, parent)
{
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)));
}

void SemaphoreHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("SemaphoreHandler::deviceStatusChanged.  deviceID: %1  status: %2").arg(deviceID).arg(status).toLatin1());

    QList<int> signalIDs;
    {
        QString sql = QString("SELECT signalID FROM signalCondition JOIN signalAspectCondition ON signalCondition.signalAspectConditionID = signalAspectCondition.id WHERE deviceID = %1").arg(deviceID);
        Database db;
        QSqlQuery query1 = db.executeQuery(sql);

        while(query1.next())
        {
            signalIDs << query1.value("signalID").toInt();
        }
    }
    for(int x = 0; x < signalIDs.count(); x++)
    {
        updateSignal(signalIDs.value(x));
    }
}

void SemaphoreHandler::newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject & /*json*/)
{
    if(uri == "/controller/module" && classCode == ClassSemaphore)
    {
        if(actionType == NetActionGet)
        {
            QList<int> signalIDs;
            {
                QString sql = QString("SELECT signal.id as signalID FROM signal JOIN controllerModule ON controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE serialNumber = %1 AND controllerModule.moduleIndex = %2 ").arg(serialNumber).arg(moduleIndex);
                Database db;
                QSqlQuery query1 = db.executeQuery(sql);

                while(query1.next())
                {
                    signalIDs << query1.value("signalID").toInt();
                }
            }
            for(int x = 0; x < signalIDs.count(); x++)
            {
                updateSignal(signalIDs.value(x));
            }
        }
    }
}

void SemaphoreHandler::updateSignal(int signalId)
{
    QString sql = QString("SELECT  signalAspectConditionID, signal.moduleIndex AS port, controllerModule.moduleIndex, serialNumber, deviceID, conditionOperand, deviceState, redMode, yellowMode, greenMode FROM signal JOIN signalAspectCondition ON signal.id = signalAspectCondition.signalID JOIN signalCondition ON signalAspectCondition.id = signalCondition.signalAspectConditionID JOIN controllerModule ON signal.controllerModuleID = controllerModule.id  JOIN controller ON controllerModule.controllerID = controller.id WHERE signalAspectCondition.signalID = %1 ORDER BY signalAspectConditionID, signalAspectCondition.sortIndex, signalCondition.sortIndex").arg(signalId);

    Database db;
    QSqlQuery query1 = db.executeQuery(sql);
    int redMode;
    int yellowMode;
    int greenMode;
    int moduleIndex;
    int serialNumber;
    int deviceState;
    int condition;
    int currentState;
    int deviceID;
    int port;
    int currentCondition = 0;

    int aspectRed = 1;
    int aspectYellow = 0;
    int aspectGreen = 0;
    bool found = true;

    while(query1.next())
    {
        if(currentCondition == 0)
        {
            currentCondition = query1.value("signalAspectConditionID").toInt();
        }
        else
        {
            if(found)
            {
                aspectRed = redMode;
                aspectYellow = yellowMode;
                aspectGreen = greenMode;
            }

            sendSignalUpdateMessage(serialNumber, moduleIndex, port, aspectRed, aspectYellow, aspectGreen);

            found = true;
            aspectRed = 1;
            aspectYellow = 0;
            aspectGreen = 0;
        }
        serialNumber = query1.value("serialNumber").toInt();
        moduleIndex = query1.value("moduleIndex").toInt();
        port = query1.value("port").toInt();
        condition = query1.value("conditionOperand").toInt();
        deviceState = query1.value("deviceState").toInt();
        redMode = query1.value("redMode").toInt();
        yellowMode = query1.value("yellowMode").toInt();
        greenMode = query1.value("greenMode").toInt();
        deviceID = query1.value("deviceID").toInt();

        currentState = DeviceManager::instance()->getDeviceStatus(deviceID);

        if(condition == ConditionEquals)
        {
            if(deviceState != currentState)
                found = false;
        }
        else if(condition == ConditionNotEquals)
        {
            if(deviceState == currentState)
                found = false;
        }
    }
    if(found)
    {
        aspectRed = redMode;
        aspectYellow = yellowMode;
        aspectGreen = greenMode;
    }

    sendSignalUpdateMessage(serialNumber, moduleIndex, port, aspectRed, aspectYellow, aspectGreen);
}

void SemaphoreHandler::sendSignalUpdateMessage(int serialNumber, int moduleIndex, int port, int redMode, int , int greenMode)
{
    int motorPinSetting = 0;

    if(redMode == 1)
        motorPinSetting = 0;
    else if(greenMode == 1)
        motorPinSetting = 1;

    QJsonObject obj;

    obj["moduleIndex"] = moduleIndex;
    obj["messageUri"] = "/controller/module";
    obj["action"] = NetActionUpdate;
    obj["port"] = port;
    obj["motorPinSetting"] = motorPinSetting;

    ControllerManager::instance()->sendMessage(serialNumber, obj);
}

