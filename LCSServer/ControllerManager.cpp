#include <QCoreApplication>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaMethod>
#include <QTimer>
#include <QPointer>

#include "ControllerManager.h"
#include "Database.h"
#include "MessageBroadcaster.h"

class ControllerEntry
{
public:
    ControllerEntry(void) : m_serialNumber(-1), m_controllerID(-1), m_status(ControllerStatusUnknown) { }
    ControllerEntry(const ControllerEntry &other) { copy(other); }

    long getSerialNumber(void) const { return m_serialNumber; }
    void setSerialNumber(long value) { m_serialNumber = value; }
    int getControllerID(void) const { return m_controllerID; }
    void setControllerID(int value) { m_controllerID = value; }
    ControllerStatusEnum getStatus(void) const { return m_status; }
    void setStatus(ControllerStatusEnum value) { m_status = value; }
    QString getVersion(void) const { return m_version; }
    void setVersion(int major, int minor, int build)
    {
        m_version = QString("%1.%2.%3").arg(major).arg(minor).arg(build);
    }
    void operator = (const ControllerEntry &other) { copy(other); }

private:
    void copy(const ControllerEntry &other)
    {
        m_serialNumber = other.m_serialNumber;
        m_controllerID = other.m_controllerID;
        m_status = other.m_status;
    }

    long m_serialNumber;
    int m_controllerID;
    ControllerStatusEnum m_status;
    QString m_version;
};

ControllerManager * ControllerManager::m_instance = NULL;

ControllerManager::ControllerManager(QObject *parent)
    : QObject(parent)
{
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(newUDPMessage(UDPMessage)));
}

ControllerManager::~ControllerManager()
{

}

ControllerManager *ControllerManager::instance()
{
    if(m_instance == NULL)
        m_instance = new ControllerManager(qApp);

    return m_instance;
}

long ControllerManager::getConnectionSerialNumber(int index) const
{
    long serialNumber(0);

    QList<long> keys = m_controllerMap.keys();

    if(m_controllerMap.value(keys.value(index)) != NULL)
        serialNumber = m_controllerMap.value(keys.value(index))->getSerialNumber();

    return serialNumber;
}

void ControllerManager::getConnectedInfo(long serialNumber, QString &version, ControllerStatusEnum &status)
{
    QList<ControllerEntry *> list = m_controllerMap.values();

    for(int x = 0; x < list.count(); x++)
    {
        if(list.value(x)->getSerialNumber() == serialNumber)
        {
            version = list.value(x)->getVersion();
            status = list.value(x)->getStatus();
            break;
        }
    }
}

unsigned long ControllerManager::getSerialNumber(int controllerID)
{
    Database db;
    return db.getSerialNumber(controllerID);
}

void ControllerManager::newUDPMessage(const UDPMessage &message)
{
    if(message.getMessageID() == SYS_CONTROLLER_ONLINE)
    {
        int controllerID = message.getID();
        int majorVersion = message.getField(5);
        int minorVersion = message.getField(6);
        int build = message.getField(7);
        long serialNumber = this->getSerialNumber(controllerID);

        ControllerEntry *entry = m_controllerMap.value(controllerID);
        if(entry == NULL)
           entry = new ControllerEntry;
        entry->setControllerID(controllerID);
        entry->setSerialNumber(serialNumber);
        entry->setStatus(ControllerStatusConected);
        entry->setVersion(majorVersion, minorVersion, build);
        m_controllerMap[controllerID] = entry;

        emit controllerAdded(serialNumber);
        emit controllerStatusChanged(serialNumber, ControllerStatusConected);
    }
    else if(message.getMessageID() == SYS_RESTARTING)
    {
        long serialNumber = message.getID();
        QList<ControllerEntry *> list = m_controllerMap.values();
        for(int x = 0; x < list.count(); x++)
        {
            if(list.value(x)->getSerialNumber() == serialNumber)
            {
                list.value(x)->setStatus(ControllerStatusRestarting);
                emit controllerStatusChanged(serialNumber, ControllerStatusRestarting);
            }
        }
    }
}
