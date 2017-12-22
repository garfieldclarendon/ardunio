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
#include "MessageBroadcaster.h"

TurnoutHandler::TurnoutHandler(QObject *parent)
    : DeviceHandler(DeviceTurnout, parent)
{
//    QTimer::singleShot(3000, this, SLOT(timerProc()));
    connect(ControllerManager::instance(), SIGNAL(controllerRemoved(int)), this, SLOT(controllerRemoved(int)));
    connect(ControllerManager::instance(), SIGNAL(messageACKed(ControllerMessage)), this, SLOT(messageACKed(ControllerMessage)));
}

void TurnoutHandler::newMessage(int serialNumber, int address, DeviceClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json)
{
    Q_UNUSED(actionType);
    if(uri == "/controller/module" && classCode == DeviceTurnout)
        updateTurnoutState(json, serialNumber, address);
    else if(uri == "/controller/module/config" && classCode == DeviceTurnout)
        sendConfig(serialNumber, address);
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
        int address;
        int port;
        int serialNumber;
        int motorPinSetting = getMotorPinSetting(deviceID);

        getIPAddressAndaddressForDevice(deviceID, ipAddress, address, port, serialNumber);
        obj["class"] = DeviceTurnout;
        obj["deviceID"] = deviceID;
        obj["newState"] = newState;
        obj["address"] = address;
        obj["messageUri"] = "/controller/module";
        obj["action"] = NetActionUpdate;
        obj["port"] = port;
        obj["motorPinSetting"] = newState == TrnNormal ? motorPinSetting : !motorPinSetting;

        ControllerMessage message(serialNumber, obj);
        ControllerManager::instance()->sendMessage(message);
    }
    UDPMessage message;
    message.setMessageID(TRN_ACTIVATE);
    message.setID(deviceID);
    message.setField(0, newState);

    MessageBroadcaster::instance()->sendUDPMessage(message);
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

int TurnoutHandler::getMotorPinSetting(int deviceID)
{
    int motorPinSetting = 0;
    QString sql = QString("SELECT deviceProperty.key, deviceProperty.value FROM deviceProperty WHERE deviceID = %1").arg(deviceID);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
         if(query.value("key").toString().toUpper() == "INPUTPIN")
            motorPinSetting = query.value("value").toInt();
    }
    return motorPinSetting;
}

void TurnoutHandler::getdeviceIDAndMotorSetting(int serialNumber, int address, int port, int &deviceID, int &motorPinSetting)
{
    motorPinSetting = 0;
    QString sql = QString("SELECT device.id, deviceProperty.key, deviceProperty.value FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerModuleID LEFT OUTER JOIN deviceProperty ON device.id = deviceProperty.deviceID WHERE controller.serialNumber = %1 AND controllerModule.address = %2 AND device.port = %3").arg(serialNumber).arg(address).arg(port);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        deviceID = query.value("id").toInt();
        if(query.value("key").toString().toUpper() == "INPUTPIN")
            motorPinSetting = query.value("value").toInt();
    }
}

void TurnoutHandler::sendConfig(int serialNumber, int address)
{
    qDebug("SENDCONFIG");
    QString queryString;
    queryString = QString("SELECT device.id, device.port, deviceProperty.value as motorPinSetting FROM device JOIN controllerModule ON device.controllerModuleID = controllerModule.id JOIN controller ON controllerModule.controllerID = controller.ID LEFT OUTER JOIN deviceProperty on device.id = deviceProperty.deviceID WHERE serialNumber = %1 AND controllerModule.address = %2 ORDER BY controllerModule.address").arg(serialNumber).arg(address);
    Database db;
    QJsonArray array = db.fetchItems(queryString);

    QString url = QString("/controller/module/config");
    QJsonObject obj;
    obj["messageUri"] = url;
    obj["action"] = NetActionUpdate;
    obj["address"] = address;
    obj["motorPinSettings"] = array;

    ControllerMessage message(serialNumber, obj);
    ControllerManager::instance()->sendMessage(message);
}

void TurnoutHandler::controllerRemoved(int serialNumber)
{
    QString sql = QString("SELECT device.id as deviceID FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerMOduleID WHERE serialNumber = %1").arg(serialNumber);

    QList<int> deviceIDs;
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        deviceIDs << query.value("deviceID").toInt();
    }

    for(int x = 0; x < deviceIDs.count(); x++)
    {
        int deviceID(deviceIDs.value(x));
        m_mapMutex.lock();
        m_turnoutStates[deviceID] = TrnUnknown;
        m_mapMutex.unlock();
        DeviceManager::instance()->setDeviceStatus(deviceID, TrnUnknown);
        createAndSendNotificationMessage(deviceID, TrnUnknown);
    }
}

void TurnoutHandler::controllerConnected(int index)
{
    // When a controller is in the process of reconnecting, we need to set the turnout state to UNKNOWN
    // so that when the controller sends the current turnout status, all panels, signals, etc will
    // get the correct state.
    int serialNumber = ControllerManager::instance()->getConnectionSerialNumber(index);
    controllerRemoved(serialNumber);
}

void TurnoutHandler::messageACKed(const ControllerMessage &message)
{
    QJsonObject obj(message.getObject());
    int controllerClass = obj["class"].toInt();
    if(controllerClass == DeviceTurnout)
    {
        int deviceID = obj["deviceID"].toInt();
        int ns = obj["newState"].toInt();
        if(ns > 0 && deviceID > 0)
        {
            TurnoutState newState = (TurnoutState)ns;
            setCurrentState(deviceID, newState == TrnNormal ? TrnToNormal : TrnToDiverging);
        }
    }
}

void TurnoutHandler::getIPAddressAndaddressForDevice(int deviceID, QString &ipAddress, int &address, int &port, int &serialNumber)
{
    QString sql = QString("SELECT serialNumber, controllerModule.address, device.port FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerMOduleID WHERE device.id = %1 AND controllerModule.disable = 0").arg(deviceID);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        serialNumber = query.value("serialNumber").toInt();
        address = query.value("address").toInt();
        port = query.value("port").toInt();
    }

    ipAddress = ControllerManager::instance()->getControllerIPAddress(serialNumber);
}

void TurnoutHandler::updateTurnoutState(const QJsonObject &json, int serialNumber, int address)
{
    qDebug(QString("updateTurnoutState %1 - %2").arg(serialNumber).arg(address).toLatin1());
    if(json.contains("turnouts"))
    {
        QJsonArray array = json["turnouts"].toArray();

        for(int x = 0; x < array.size(); x++)
        {
            QJsonObject obj = array.at(x).toObject();
            int port = obj["port"].toInt();

            int deviceID;
            int motorPinSetting;

            TurnoutState turnoutState;

            getdeviceIDAndMotorSetting(serialNumber, address, port, deviceID, motorPinSetting);

            turnoutState = getTurnoutState(obj, motorPinSetting);

            if(deviceID > 0 && turnoutState != TrnUnknown)
            {
                qDebug(QString("UPDATING TURNOUT: %1 NewState %2 pinSetting %3").arg(deviceID).arg(turnoutState).arg(motorPinSetting).toLatin1());
                setCurrentState(deviceID, turnoutState);
            }
        }
    }
}

void TurnoutHandler::setCurrentState(int deviceID, TurnoutState newState)
{
    m_mapMutex.lock();
    TurnoutState current = m_turnoutStates.value(deviceID);
    m_mapMutex.unlock();
    if(current != newState && newState != TrnUnknown)
    {
        if(current == TrnToDiverging || current == TrnToNormal)
        {
            if((current == TrnToNormal && newState == TrnNormal) || (current == TrnToDiverging && newState == TrnDiverging))
            {
                m_mapMutex.lock();
                m_turnoutStates[deviceID] = newState;
                m_mapMutex.unlock();
                DeviceManager::instance()->setDeviceStatus(deviceID, newState);
                createAndSendNotificationMessage(deviceID, newState);
            }
        }
        else
        {
            m_mapMutex.lock();
            m_turnoutStates[deviceID] = newState;
            m_mapMutex.unlock();
            DeviceManager::instance()->setDeviceStatus(deviceID, newState);
            createAndSendNotificationMessage(deviceID, newState);
        }
    }
}

void TurnoutHandler::createAndSendNotificationMessage(int deviceID, TurnoutState newState)
{
    QString uri("/api/notification/turnout");
    QJsonObject obj;
    obj["deviceID"] = QString("%1").arg(deviceID);
    obj["deviceState"] = QString("%1").arg(newState);

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


