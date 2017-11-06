#include <QList>
#include <QSqlQuery>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>

#include "SignalHandler.h"
#include "Database.h"
#include "DeviceManager.h"
#include "ControllerManager.h"
#include "WebServer.h"

SignalHandler::SignalHandler(QObject *parent)
    : DeviceHandler(ClassSignal, parent)
{
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)), Qt::QueuedConnection);
}

void SignalHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("SignalHandler::deviceStatusChanged.  deviceID: %1  status: %2").arg(deviceID).arg(status).toLatin1());

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

void SignalHandler::newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject & /*json*/)
{
    if(uri == "/controller/module" && classCode == ClassSignal)
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

void SignalHandler::updateSignal(int deviceID)
{
    QString sql = QString("SELECT DISTINCT signalAspectID, device.port, controllerModule.address, serialNumber, signalCondition.deviceID, conditionOperand, deviceState, redMode, yellowMode, greenMode FROM device JOIN signalAspect ON device.id = signalAspect.deviceID JOIN signalCondition ON signalAspect.id = signalCondition.signalAspectID JOIN controllerModule ON device.controllerModuleID = controllerModule.id  JOIN controller ON controllerModule.controllerID = controller.id WHERE signalAspect.deviceID = %1 AND device.deviceClass = 4 AND controllerModule.disable = 0 ORDER BY signalAspect.sortIndex, signalAspectID").arg(deviceID);

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
        // When currentCondition changes we know we've gone through all of the conditions for this signal aspect entry.  If "found" is TRUE,
        // then all of the conditions past their tests which means we should set the signal to this aspect....send out the
        // signal aspect defined in this entry.  If any of the conditions fails the test, then we skip this entry and go on to the next
        if(currentCondition != query1.value("signalAspectID").toInt())
        {
            // The first time through, currentCondition will be 0 so we can skip sending the update
            if(currentCondition != 0)
            {
                // If we've gone through all of the conditions for this signal entry and found is still TRUE, then this is the aspect
                // we should use for the signal so break out of the loop
                if(found)
                    break;
            }

            // if we get this far then at least one of the condition entries for this aspect failed the test so we need to reset
            // for the next signal aspect entry.
            found = true;
        }

        // Only process the remaining conditions in this aspect entry if "found" is still true.  If it's already failed a a test
        // skip the remaining conditions
        if(found)
        {
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
        // before going to the next record, get the signalAspectID and save it in the currentCondition variable.  This gets
        // tested above.  When this id changes, we know we've processed all of the conditions for this signal aspect.
        currentCondition = query1.value("signalAspectID").toInt();
    }

    // By this point we either found an entry that passed all of the condition tests in which case, "found" will be true which means we should use this
    // entries LED settings for this signal.  If found is false at this point then we've gone through all of the possible signal aspect entries
    // without a match.  In this case we default to RED or the most restrictive aspect for the signal.  This implies that you only need to enter
    // entries in the database of non-RED signal aspects.
    if(found)
    {
        aspectRed = redMode;
        aspectYellow = yellowMode;
        aspectGreen = greenMode;
    }

    if(serialNumber > 0)
        sendSignalUpdateMessage(serialNumber, address, port, aspectRed, aspectYellow, aspectGreen);
}

void SignalHandler::sendSignalUpdateMessage(int serialNumber, int address, int port, int redMode, int yellowMode, int greenMode)
{
    QJsonObject obj;
    QJsonObject pin1, pin2, pin3;
    QJsonArray pins;

    obj["address"] = address;
    obj["messageUri"] = "/controller/module";
    obj["action"] = NetActionUpdate;

    pin1["pin"] = port;
    pin1["pinState"] = redMode;

    pin2["pin"] = port + 1;
    pin2["pinState"] = greenMode;

    if(port + 2 < 16)
    {
        pin3["pin"] = port + 2;
        pin3["pinState"] = yellowMode;
        pins << pin1 << pin2 << pin3;
    }

    obj["pins"] = pins;

    ControllerMessage message(serialNumber, obj);
    ControllerManager::instance()->sendMessage(message);
}

