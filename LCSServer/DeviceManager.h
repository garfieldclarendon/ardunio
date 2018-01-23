#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <QDateTime>

#include "GlobalDefs.h"
#include "UDPMessage.h"

class DeviceHandler;

class DeviceStatus
{
public:
    DeviceStatus(void)
        : m_locked(false), m_currentStatus(0)
    {

    }

    DeviceStatus(bool locked, int status, const QDateTime &dateTime)
        : m_locked(locked), m_currentStatus(status), m_statusDateTime(dateTime)
    {

    }

    DeviceStatus(const DeviceStatus &other)
    {
        m_locked = other.m_locked;
        m_currentStatus = other.m_currentStatus;
        m_statusDateTime = other.m_statusDateTime;
    }

    void operator = (const DeviceStatus &other)
    {
        m_locked = other.m_locked;
        m_currentStatus = other.m_currentStatus;
        m_statusDateTime = other.m_statusDateTime;
    }

    int getCurrentStatus(void) const { return m_currentStatus; }
    void setCurrentStatus(int value) { m_currentStatus = value; }
    QDateTime getStatusDateTime(void) const { return m_statusDateTime; }
    void setStatusDateTime(const QDateTime &value) { m_statusDateTime = value; }
    bool getLocked(void) const { return m_locked; }
    void setLocked(bool value) { m_locked = value; }

private:
    bool m_locked;
    int m_currentStatus;
    QDateTime m_statusDateTime;
};

class DeviceManager : public QObject
{
    Q_OBJECT
    explicit DeviceManager(QObject *parent = 0);
public:
    ~DeviceManager(void);
    static DeviceManager *instance(void);

    bool getIsDeviceLocked(int deviceID);
    int getDeviceStatus(int deviceID);
    int getDeviceCount(void) const;
    int getDeviceID(int index) const;

signals:
//    void deviceStatusChanged(ClassEnum classCode);
    void deviceStatusChanged(int deviceID, int status, bool locked);

public slots:
    DeviceHandler *getHandler(DeviceClassEnum classCode) const { return m_deviceMap.value(classCode); }
    void setDeviceStatus(bool locked, int deviceID, int status);
    void newUDPMessage(const UDPMessage &message);

protected:
    void addDeviceHandler(DeviceClassEnum classCode, DeviceHandler *handler);
    void removeDeviceHandler(DeviceClassEnum classCode);

private:
    static DeviceManager *m_instance;

    QMap<DeviceClassEnum, DeviceHandler *> m_deviceMap;
    QMap<int, DeviceStatus> m_statusMap;

    friend class DeviceHandler;
};

#endif // DEVICEMANAGER_H
