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
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)));
}

void SignalHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("SignalHandler::deviceStatusChanged.  deviceID: %1  status: %2").arg(deviceID).arg(status).toLatin1());

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

void SignalHandler::newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject & /*json*/)
{
    if(uri == "/controller/module" && classCode == ClassSignal)
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

void SignalHandler::updateSignal(int signalId)
{
    QString sql = QString("SELECT  signalAspectConditionID, signal.moduleIndex AS port, controllerModule.moduleIndex, serialNumber, deviceID, conditionOperand, deviceState, redMode, yellowMode, greenMode FROM signal JOIN signalAspectCondition ON signal.id = signalAspectCondition.signalID JOIN signalCondition ON signalAspectCondition.id = signalCondition.signalAspectConditionID JOIN controllerModule ON signal.controllerModuleID = controllerModule.id  JOIN controller ON controllerModule.controllerID = controller.id WHERE signalAspectCondition.signalID = %1 AND controllerModule.moduleClass = 4 ORDER BY signalAspectConditionID, signalAspectCondition.sortIndex, signalCondition.sortIndex").arg(signalId);

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
        // When currentCondition changes we know we've gone through all of the conditions for this signal aspect entry.  If "found" is TRUE,
        // then all of the conditions past their tests which means we should set the signal to this aspect....send out the
        // signal aspect defined in this entry.  If any of the conditions fails the test, then we skip this entry and go on to the next
        if(currentCondition != query1.value("signalAspectConditionID").toInt())
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
        // before going to the next record, get the signalAspectConditionID and save it in the currentCondition variable.  This gets
        // tested above.  When this id changes, we know we've processed all of the conditions for this signal aspect.
        currentCondition = query1.value("signalAspectConditionID").toInt();
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

    sendSignalUpdateMessage(serialNumber, moduleIndex, port, aspectRed, aspectYellow, aspectGreen);
}

void SignalHandler::sendSignalUpdateMessage(int serialNumber, int moduleIndex, int port, int redMode, int yellowMode, int greenMode)
{
    QJsonObject obj;

    obj["moduleIndex"] = moduleIndex;
    obj["messageUri"] = "/controller/module";
    obj["action"] = NetActionUpdate;
    obj["port"] = port;
    obj["pin1State"] = redMode;
    obj["pin2State"] = greenMode;
    obj["pin3State"] = yellowMode;

    ControllerMessage message(serialNumber, obj);
    ControllerManager::instance()->sendMessage(message);
}

