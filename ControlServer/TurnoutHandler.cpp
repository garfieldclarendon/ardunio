#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QVariant>
#include <QTimer>

#include "TurnoutHandler.h"
#include "Database.h"
#include "ControllerManager.h"
#include "DeviceManager.h"
#include "WebServer.h"

TurnoutHandler::TurnoutHandler(QObject *parent)
    : DeviceHandler(ClassTurnout, parent)
{
//    QTimer::singleShot(3000, this, SLOT(timerProc()));
}

void TurnoutHandler::newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json)
{
    Q_UNUSED(actionType);
    if(uri == "/controller/module" && classCode == ClassTurnout)
        updateTurnoutState(json, serialNumber, moduleIndex);
    else if(uri == "/controller/module/config" && classCode == ClassTurnout)
        sendConfig(serialNumber, moduleIndex);
}

void TurnoutHandler::activateTurnout(int deviceID, TurnoutState newState)
{
    TurnoutState current = m_turnoutStates.value(deviceID);

    if(current != newState)
    {
        if(newState == TrnNormal)
        {
            qDebug("NORMAL!!!");
        }
        QJsonObject obj;

        QString ipAddress;
        int moduleIndex;
        int port;
        int serialNumber;
        int motorPinSetting = getMotorPinSetting(deviceID);

        getIPAddressAndModuleIndexForDevice(deviceID, ipAddress, moduleIndex, port, serialNumber);
        obj["moduleIndex"] = moduleIndex;
        obj["messageUri"] = "/controller/module";
        obj["action"] = NetActionUpdate;
        obj["port"] = port;
        obj["motorPinSetting"] = newState == TrnNormal ? motorPinSetting : !motorPinSetting;

        ControllerManager::instance()->sendMessage(serialNumber, obj);
        setCurrentState(deviceID, newState == TrnNormal ? TrnToNormal : TrnToDiverging);
    }
}

void TurnoutHandler::timerProc()
{
    QMap<int, TurnoutState> mapCopy;
    m_mapMutex.lock();
    mapCopy = m_turnoutStates;
    mapCopy.detach();
    m_mapMutex.unlock();

    QMapIterator<int, TurnoutState> i(mapCopy);
    while(i.hasNext())
    {
        i.next();
        if(i.value() == TrnToDiverging || i.value() == TrnToNormal)
        {
            activateTurnout(i.key(), i.value() == TrnToDiverging ? TrnDiverging : TrnNormal);
        }
    }
    QTimer::singleShot(3000, this, SLOT(timerProc()));
}

int TurnoutHandler::getMotorPinSetting(int turnoutID)
{
    int motorPinSetting = 0;
    QString sql = QString("SELECT deviceProperty.key, deviceProperty.value FROM deviceProperty WHERE deviceID = %1").arg(turnoutID);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
         if(query.value("key").toString().toUpper() == "INPUTPIN")
            motorPinSetting = query.value("value").toInt();
    }
    return motorPinSetting;
}

void TurnoutHandler::getTurnoutIDAndMotorSetting(int serialNumber, int moduleIndex, int port, int &turnoutID, int &motorPinSetting)
{
    motorPinSetting = 0;
    QString sql = QString("SELECT device.id, deviceProperty.key, deviceProperty.value FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerModuleID LEFT OUTER JOIN deviceProperty ON device.id = deviceProperty.deviceID WHERE controller.serialNumber = %1 AND controllerModule.moduleIndex = %2 AND device.moduleIndex = %3").arg(serialNumber).arg(moduleIndex).arg(port);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        turnoutID = query.value("id").toInt();
        if(query.value("key").toString().toUpper() == "INPUTPIN")
            motorPinSetting = query.value("value").toInt();
    }
}

void TurnoutHandler::sendConfig(int serialNumber, int moduleIndex)
{
    qDebug("SENDCONFIG");
    QString queryString;
    queryString = QString("SELECT device.id, device.moduleIndex as port, deviceProperty.value as motorPinSetting FROM device JOIN controllerModule ON device.controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.ID LEFT OUTER JOIN deviceProperty on device.id = deviceProperty.deviceID WHERE serialNumber = %1 AND controllerModule.moduleIndex = %2 ORDER BY controllerModule.moduleIndex").arg(serialNumber).arg(moduleIndex);
    Database db;
    QJsonArray array = db.fetchItems(queryString);

    QString url = QString("/controller/module/config");
    QJsonObject obj;
    obj["messageUri"] = url;
    obj["action"] = NetActionUpdate;
    obj["moduleIndex"] = moduleIndex;
    obj["motorPinSettings"] = array;

    ControllerManager::instance()->sendMessage(serialNumber, obj);
}

void TurnoutHandler::getIPAddressAndModuleIndexForDevice(int deviceID, QString &ipAddress, int &moduleIndex, int &port, int &serialNumber)
{
    QString sql = QString("SELECT serialNumber, controllerModule.moduleIndex, device.moduleIndex as port FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerMOduleID WHERE device.id = %1").arg(deviceID);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        serialNumber = query.value("serialNumber").toInt();
        moduleIndex = query.value("moduleIndex").toInt();
        port = query.value("port").toInt();
    }

    ipAddress = ControllerManager::instance()->getControllerIPAddress(serialNumber);
}

void TurnoutHandler::updateTurnoutState(const QJsonObject &json, int serialNumber, int moduleIndex)
{
    qDebug(QString("updateTurnoutState %1 - %2").arg(serialNumber).arg(moduleIndex).toLatin1());
    if(json.contains("turnouts"))
    {
        QJsonArray array = json["turnouts"].toArray();

        for(int x = 0; x < array.size(); x++)
        {
            QJsonObject obj = array.at(x).toObject();
            int port = obj["port"].toInt();

            int turnoutID;
            int motorPinSetting;

            TurnoutState turnoutState;

            getTurnoutIDAndMotorSetting(serialNumber, moduleIndex, port, turnoutID, motorPinSetting);

            turnoutState = getTurnoutState(obj, motorPinSetting);

            if(turnoutID > 0 && turnoutState != TrnUnknown)
            {
                qDebug(QString("UPDATING TURNOUT: %1 NewState %2 pinSetting %3").arg(turnoutID).arg(turnoutState).arg(motorPinSetting).toLatin1());
                setCurrentState(turnoutID, turnoutState);
            }
        }
    }
}

void TurnoutHandler::setCurrentState(int turnoutID, TurnoutState newState)
{
    m_mapMutex.lock();
    TurnoutState current = m_turnoutStates.value(turnoutID);
    m_mapMutex.unlock();
    if(current != newState && newState != TrnUnknown)
    {
        if(current == TrnToDiverging || current == TrnToNormal)
        {
            if((current == TrnToNormal && newState == TrnNormal) || (current == TrnToDiverging && newState == TrnDiverging))
            {
                m_mapMutex.lock();
                m_turnoutStates[turnoutID] = newState;
                m_mapMutex.unlock();
                DeviceManager::instance()->setDeviceStatus(turnoutID, newState);
                createAndSendNotificationMessage(turnoutID, newState);
            }
        }
        else
        {
            m_mapMutex.lock();
            m_turnoutStates[turnoutID] = newState;
            m_mapMutex.unlock();
            DeviceManager::instance()->setDeviceStatus(turnoutID, newState);
            createAndSendNotificationMessage(turnoutID, newState);
        }
    }
}

void TurnoutHandler::createAndSendNotificationMessage(int turnoutID, TurnoutState newState)
{
    QString uri("/api/notification/turnout");
    QJsonObject obj;
    obj["turnoutID"] = turnoutID;
    obj["state"] = newState;

    emit sendNotificationMessage(uri, obj);
}

TurnoutState TurnoutHandler::getTurnoutState(const QJsonObject &obj, int motorPinSetting) const
{
    TurnoutState turnoutState = TrnUnknown;

    int feedbackA = obj["feedbackA"].toInt();
    int feedbackB = obj["feedbackB"].toInt();

    if(feedbackA == 0 && feedbackB == 1)
        turnoutState = motorPinSetting == 0 ? TrnNormal : TrnDiverging;
    else if(feedbackA == 1 && feedbackB == 0)
        turnoutState = motorPinSetting == 0 ? TrnDiverging : TrnNormal;

    return turnoutState;
}
