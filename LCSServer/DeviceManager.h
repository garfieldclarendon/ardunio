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
        : m_currentStatus(0)
    {

    }

    DeviceStatus(int status, const QDateTime &dateTime)
        : m_currentStatus(status), m_statusDateTime(dateTime)
    {

    }

    DeviceStatus(const DeviceStatus &other)
    {
        m_currentStatus = other.m_currentStatus;
        m_statusDateTime = other.m_statusDateTime;
    }

    void operator = (const DeviceStatus &other)
    {
        m_currentStatus = other.m_currentStatus;
        m_statusDateTime = other.m_statusDateTime;
    }

    int getCurrentStatus(void) const { return m_currentStatus; }
    void setCurrentStatus(int value) { m_currentStatus = value; }
    QDateTime getStatusDateTime(void) const { return m_statusDateTime; }
    void setStatusDateTime(const QDateTime &value) { m_statusDateTime = value; }

private:
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

    int getDeviceStatus(int deviceID);
    int getDeviceCount(void) const;
    int getDeviceID(int index) const;

signals:
//    void deviceStatusChanged(ClassEnum classCode);
    void deviceStatusChanged(int deviceID, int status);
    void sendNotificationMessage(const QString &uri, const QJsonObject &obj);

public slots:
    DeviceHandler *getHandler(DeviceClassEnum classCode) const { return m_deviceMap.value(classCode); }
    void setDeviceStatus(int deviceID, int status);
    void newUDPMessage(const UDPMessage &message);

protected:
    void addDeviceHandler(DeviceClassEnum classCode, DeviceHandler *handler);
    void removeDeviceHandler(DeviceClassEnum classCode);

private:
    void createAndSendNotificationMessage(int deviceID, int newState);
    static DeviceManager *m_instance;

    QMap<DeviceClassEnum, DeviceHandler *> m_deviceMap;
    QMap<int, DeviceStatus> m_statusMap;

    friend class DeviceHandler;
};

#endif // DEVICEMANAGER_H