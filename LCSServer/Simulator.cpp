#include <QTimer>

#include "Simulator.h"
#include "MessageBroadcaster.h"
#include "Database.h"

Simulator::Simulator(QObject *parent) : QObject(parent)
{
    loadData();
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(newMessage(UDPMessage)));
    QTimer::singleShot(1000, this, SLOT(timerProc()));
}

void Simulator::newMessage(const UDPMessage &message)
{
    if(message.getMessageID() == TRN_ACTIVATE)
    {
        int deviceID = message.getSerialNumber();
        TurnoutState newState = (TurnoutState)message.getField(0);
        activateTurnout(deviceID, newState);
    }
    else if(message.getMessageID() == TRN_ACTIVATE_ROUTE)
    {
        handleActivateRouteMessage(message);
    }
    else if(message.getMessageID() == SYS_CONTROLLER_ONLINE)
    {
        for(int x = 0; x < m_array.count(); x++)
        {
//            QJsonObject obj = m_array[x].toObject();
//            int deviceID = obj["deviceID"].toVariant().toInt();
//            int status = obj["currentState"].toVariant().toInt();
//            sendStatus(deviceID, (TurnoutState)status);
        }
    }
}

void Simulator::timerProc()
{
    for(int x = 0; x < m_array.count(); x++)
    {
        QJsonObject obj = m_array[x].toObject();
        int timeout = obj["timeout"].toVariant().toInt();
        int status = obj["currentState"].toVariant().toInt();

        if(timeout >= 1)
        {
            int deviceID = obj["deviceID"].toVariant().toInt();

            if(status == TrnToDiverging)
                status = TrnDiverging;
            else if(status == TrnToNormal)
                status = TrnNormal;

            obj["currentState"] = status;
            obj["timeout"] = 0;
            m_array [x] = obj;
            sendStatus(deviceID, (TurnoutState)status);
        }
        else if(status == TrnToDiverging || status == TrnToNormal)
        {
            obj["timeout"] = timeout + 1;
            m_array [x] = obj;
        }
    }

    QTimer::singleShot(1000, this, SLOT(timerProc()));
}

void Simulator::loadData()
{
    Database db;
    m_array = db.fetchItems("SELECT device.id as deviceID, deviceName FROM device JOIN controllerModule ON device.controllerModuleID = controllerModule.id WHERE disable = 0 AND deviceClass = 1");
    for(int x = 0; x < m_array.count(); x++)
    {
        QJsonObject obj = m_array[x].toObject();
        int deviceID = obj["deviceID"].toVariant().toInt();
        obj["currentState"] = TrnNormal;
        obj["timeout"] = 0;
        m_array[x] = obj;

        sendStatus(deviceID, TrnNormal);
    }
}

void Simulator::handleActivateRouteMessage(const UDPMessage &message)
{
    int routeID = message.getSerialNumber();

    QString sql = QString("SELECT id, deviceID, turnoutState FROM routeEntry WHERE routeID = %1").arg(routeID);
    Database db;
    QJsonArray array = db.fetchItems(sql);

    for(int x = 0; x < array.count(); x++)
    {
        QJsonObject obj = array[x].toObject();
        int deviceID = obj["deviceID"].toVariant().toInt();
        TurnoutState newState = (TurnoutState)obj["turnoutState"].toVariant().toInt();

        activateTurnout(deviceID, newState);
    }
}

void Simulator::activateTurnout(int deviceID, TurnoutState newState)
{
    for(int x = 0; x < m_array.count(); x++)
    {
        QJsonObject obj = m_array[x].toObject();

        if(obj["deviceID"].toVariant().toInt() == deviceID)
        {
            activateTurnout(obj, newState);
            m_array[x] = obj;
            break;
        }
    }
}

void Simulator::activateTurnout(QJsonObject &obj, TurnoutState newState)
{
    if(obj["currentState"].toVariant().toInt() != newState)
    {
        if(newState == TrnDiverging)
            obj["currentState"] = TrnToDiverging;
        else if(newState == TrnNormal)
            obj["currentState"] = TrnToNormal;

        obj["timeout"] = 0;
        sendStatus(obj["deviceID"].toVariant().toInt(), (TurnoutState)obj["currentState"].toVariant().toInt());
    }
}

void Simulator::sendStatus(int deviceID, TurnoutState currentState)
{
    UDPMessage message;
    message.setMessageID(TRN_STATUS);
    message.setSerialNumber(deviceID);
    message.setField(0, currentState);
    MessageBroadcaster::instance()->sendUDPMessage(message);
}
