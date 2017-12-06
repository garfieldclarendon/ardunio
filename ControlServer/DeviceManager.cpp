#include <QCoreApplication>
#include <QJsonObject>

#include "DeviceManager.h"
#include "DeviceHandler.h"
#include "MessageBroadcaster.h"
#include "NotificationServer.h"

DeviceManager * DeviceManager::m_instance = NULL;

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(newUDPMessage(UDPMessage)));
    connect(this, &DeviceManager::sendNotificationMessage, NotificationServer::instance(), &NotificationServer::sendNotificationMessage);
}

DeviceManager::~DeviceManager()
{
}

DeviceManager *DeviceManager::instance()
{
    if(m_instance == NULL)
        m_instance = new DeviceManager(qApp);

    return m_instance;
}

int DeviceManager::getDeviceStatus(int deviceID)
{
    return m_statusMap.value(deviceID).getCurrentStatus();
}

int DeviceManager::getDeviceCount(void) const
{
    return m_statusMap.keys().count();
}

int DeviceManager::getDeviceID(int index) const
{
    return m_statusMap.keys().value(index);
}

void DeviceManager::setDeviceStatus(int deviceID, int status)
{
    DeviceStatus newStatus(status, QDateTime::currentDateTime());

    if(m_statusMap.value(deviceID).getCurrentStatus() != status)
    {
        qDebug(QString("!!!!!!!!!!!!!!!SET DEVICE STATUS %1 to %2").arg(deviceID).arg(status).toLatin1());
        m_statusMap[deviceID] = newStatus;
        emit deviceStatusChanged(deviceID, status);
        createAndSendNotificationMessage(deviceID, status);
    }
}

void DeviceManager::newUDPMessage(const UDPMessage &message)
{
    if(message.getMessageID() == TRN_STATUS)
    {
        int deviceID = message.getSerialNumber();
        int state = message.getField(0);
        setDeviceStatus(deviceID, state);
    }
}

void DeviceManager::addDeviceHandler(DeviceClassEnum classCode, DeviceHandler *handler)
{
    m_deviceMap[classCode] = handler;
}

void DeviceManager::removeDeviceHandler(DeviceClassEnum classCode)
{
    m_deviceMap.remove(classCode);
}

void DeviceManager::createAndSendNotificationMessage(int deviceID, int newState)
{
    QString uri("/api/notification/device");
    QJsonObject obj;
    obj["deviceID"] = QString("%1").arg(deviceID);
    obj["deviceState"] = QString("%1").arg(newState);

    emit sendNotificationMessage(uri, obj);
}
