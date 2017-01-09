#include <QUuid>
#include <QByteArray>

#include "SystemMessageHandler.h"
#include "UDPMessage.h"
#include "Database.h"
#include "GlobalDefs.h"
#include "MessageBroadcaster.h"

SystemMessageHandler::SystemMessageHandler(QObject *parent) : QObject(parent)
{

}

void SystemMessageHandler::handleMessage(const UDPMessage &message)
{
    if(message.getMessageID() == SYS_NEW_CONTROLLER)
        registerController(message);
}

void SystemMessageHandler::registerController(const UDPMessage &message)
{
    long serialNumber(message.getLValue());
    qDebug(QString("Getting ControllerID for SerialNumber %2").arg(serialNumber).toLatin1());

    Database db;
    int deviceID = db.getControllerID(serialNumber);

    if(deviceID > 0)
    {
        UDPMessage newMessage;
        newMessage.setMessageID(SYS_SET_CONTROLLER_ID);
        newMessage.setMessageClass(ClassSystem);
        newMessage.setControllerID(deviceID);
        newMessage.setLValue(serialNumber);
        newMessage.setMessageVersion(1);

        MessageBroadcaster::instance()->sendUDPMessage(newMessage);
    }
}

void SystemMessageHandler::createNewMacAddress(quint8 *mac)
{
    QUuid guid(QUuid::createUuid());

    QByteArray ba(guid.toByteArray());

    mac[0] = ba[1];
    mac[1] = ba[2];
    mac[2] = ba[3];
    mac[3] = ba[4];
    mac[4] = ba[5];
    mac[5] = ba[6];
}
