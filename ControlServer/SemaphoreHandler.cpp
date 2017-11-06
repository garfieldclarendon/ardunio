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
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)), Qt::QueuedConnection);
}

void SemaphoreHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("SemaphoreHandler::deviceStatusChanged.  deviceID: %1  status: %2").arg(deviceID).arg(status).toLatin1());

    QList<int> deviceIDs;
    {
        QString sql = QString("SELECT DISTINCT signalAspect.deviceID FROM signalCondition JOIN signalAspect ON signalCondition.signalAspectID = signalAspect.id WHERE signalCondition.deviceID = %1").arg(deviceID);
        Database db;
        QSqlQuery query1 = db.executeQuery(sql);

        while(query1.next())
        {
            deviceIDs << query1.value("deviceID").toInt();
        }
    }
    for(int x = 0; x < deviceIDs.count(); x++)
    {
        updateSignal(deviceIDs.value(x));
    }
}

void SemaphoreHandler::newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject & /*json*/)
{
    if(uri == "/controller/module" && classCode == ClassSemaphore)
    {
        if(actionType == NetActionGet)
        {
            QList<int> deviceIDs;
            {
                QString sql = QString("SELECT device.id as deviceID FROM device JOIN controllerModule ON controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.id WHERE serialNumber = %1 AND controllerModule.address = %2 AND controllerModule.disable = 0").arg(serialNumber).arg(address);
                Database db;
                QSqlQuery query1 = db.executeQuery(sql);

                while(query1.next())
                {
                    deviceIDs << query1.value("deviceID").toInt();
                }
            }
            for(int x = 0; x < deviceIDs.count(); x++)
            {
                updateSignal(deviceIDs.value(x));
            }
        }
    }
}

void SemaphoreHandler::updateSignal(int deviceID)
{
    QString sql = QString("SELECT signalAspectID, device.port, controllerModule.address, serialNumber, signalCondition.deviceID, conditionOperand, deviceState, redMode, yellowMode, greenMode FROM device JOIN signalAspect ON device.id = signalAspect.deviceID JOIN signalCondition ON signalAspect.id = signalCondition.signalAspectID JOIN controllerModule ON device.controllerModuleID = controllerModule.id  JOIN controller ON controllerModule.controllerID = controller.id WHERE signalAspect.deviceID = %1 AND device.deviceClass = 5 AND controllerModule.disable = 0 ORDER BY signalAspect.sortIndex, signalAspectID").arg(deviceID);

    Database db;
    QSqlQuery query1 = db.executeQuery(sql);
    int redMode = 0;
    int yellowMode = 0;
    int greenMode = 0;
    int address;
    int serialNumber = 0;
    int deviceState;
    int condition;
    int currentState;
    int id;
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
            currentCondition = query1.value("signalAspectID").toInt();
        }
        else
        {
            if(found)
            {
                aspectRed = redMode;
                aspectYellow = yellowMode;
                aspectGreen = greenMode;
            }

            sendSignalUpdateMessage(serialNumber, address, port, aspectRed, aspectYellow, aspectGreen);

            found = true;
            aspectRed = 1;
            aspectYellow = 0;
            aspectGreen = 0;
        }
        serialNumber = query1.value("serialNumber").toInt();
        address = query1.value("address").toInt();
        port = query1.value("port").toInt();
        condition = query1.value("conditionOperand").toInt();
        deviceState = query1.value("deviceState").toInt();
        redMode = query1.value("redMode").toInt();
        yellowMode = query1.value("yellowMode").toInt();
        greenMode = query1.value("greenMode").toInt();
        id = query1.value("deviceID").toInt();

        currentState = DeviceManager::instance()->getDeviceStatus(id);

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

    if(serialNumber > 0)
        sendSignalUpdateMessage(serialNumber, address, port, aspectRed, aspectYellow, aspectGreen);
}

void SemaphoreHandler::sendSignalUpdateMessage(int serialNumber, int address, int port, int redMode, int , int greenMode)
{
    int motorPinSetting = 0;

    if(redMode == 1)
        motorPinSetting = 0;
    else if(greenMode == 1)
        motorPinSetting = 1;

    QJsonObject obj;

    obj["address"] = address;
    obj["messageUri"] = "/controller/module";
    obj["action"] = NetActionUpdate;
    obj["port"] = port;
    obj["motorPinSetting"] = motorPinSetting;

    ControllerMessage message(serialNumber, obj);
    ControllerManager::instance()->sendMessage(message);
}

