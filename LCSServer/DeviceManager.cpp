#include <QCoreApplication>
#include <QJsonObject>

#include "DeviceManager.h"
#include "DeviceHandler.h"
#include "MessageBroadcaster.h"

DeviceManager * DeviceManager::m_instance = NULL;

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
{
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(newUDPMessage(UDPMessage)));
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

bool DeviceManager::getIsDeviceLocked(int deviceID)
{
    return m_statusMap.value(deviceID).getLocked();
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

void DeviceManager::setDeviceStatus(bool locked, int deviceID, int status)
{
    DeviceStatus newStatus(locked, status, QDateTime::currentDateTime());

    if(m_statusMap.value(deviceID).getCurrentStatus() != status || m_statusMap.value(deviceID).getLocked() != locked)
    {
        qDebug(QString("!!!!!!!!!!!!!!!SET DEVICE STATUS %1 to %2").arg(deviceID).arg(status).toLatin1());
        m_statusMap[deviceID] = newStatus;
        emit deviceStatusChanged(deviceID, status, locked);
    }
}

void DeviceManager::newUDPMessage(const UDPMessage &message)
{
    if(message.getMessageID() == TRN_STATUS || message.getMessageID() == BLK_STATUS)
    {
        int deviceID = message.getID();
        int state = message.getField(0);
        bool locked = message.getField(1);

        setDeviceStatus(locked, deviceID, state);
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
