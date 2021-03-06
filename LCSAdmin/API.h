#ifndef API_H
#define API_H

#include <QObject>
#include <QWebSocket>
#include <QUdpSocket>
#include <QTimer>

#include "GlobalDefs.h"
#include "Entity.h"

class UDPMessage;

class API : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QString serverAddress READ getServerAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(int serverPort READ getServerPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(bool apiReady READ getApiReady NOTIFY apiReady)
    Q_PROPERTY(QStringList serverList READ getServerList NOTIFY serverListChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

    explicit API(const QString &server, const int port, QObject *parent = nullptr);
    explicit API(QObject *parent = nullptr);
    ~API(void);

    static API *instance(void);

    QString getServerAddress(void) const { return m_server; }
    void setServerAddress(const QString &value);
    int getServerPort(void) const { return m_port; }
    void setServerPort(int value) { m_port = value; emit serverPortChanged(); }
    bool busy(void) const { return m_busy; }
    void setBusy(bool value);

signals:
    void controllerChanged(int serialNumber, ControllerStatusEnum status, const QString &version);
    void deviceChanged(int deviceID, int status);
    void apiReady(void);
    void serverAddressChanged(void);
    void serverPortChanged(void);
    void routeChanged(int routeID, bool isActive, bool isLocked, bool canLock);
    void serverListChanged(void);
    void busyChanged(void);

public slots:
    void activateTurnout(int deviceID, int newState);
    void activateRoute(int routeID);
    void lockRoute(int routeID, bool lock);
    QString getControllerList(int controllerID);
    QString getControllerModuleListByControllerID(int controllerID);
    QString getControllerModuleListByModuleID(int controllerModuleID);
    QString getControllerModuleListByClass(ModuleClassEnum moduleClass);
    QString getDeviceList(int controllerID, int moduleID, DeviceClassEnum deviceType = DeviceUnknown, int deviceID = -1);
    QString getModuleDevicePortList(int deviceID, int moduleID, int controllerID);
    QString getSignalAspectList(int deviceID);
    QString getSignalConditionList(int aspectID);
    QString getRouteList(int deviceID);
    QString getRouteEntryList(int routeID);
    QString getDevicePropertyList(int deviceID);
    QString createNewDevice(const QString &name, const QString &description, DeviceClassEnum deviceClass, bool createExtra);

    Entity saveEntity(const Entity &entity, bool isNew = false);
    Entity deleteEntity(const Entity &entity);

    bool getApiReady(void);

    void restartController(int serialNumber);
    void sendControllerConfig(int serialNumber);
    void sendControllerNotificationList(int serialNumber);
    void sendDeviceConfig(int deviceID);
    void sendFirmware(int serialNumber);

    QStringList getServerList(void) const;
    void addServerToList(const QString &address);

protected slots:
    void textMessageReceived(const QString &message);
    void notificationStateChanged(QAbstractSocket::SocketState state);
    void connectNotificationSocket(void);
    void newUDPMessage(const UDPMessage &message);
    void findServerSlot(void);

private:
    void setupNotificationSocket(void);
    void setupUDPSocket(void);
    QUrl buildNotifcationUrl(const QString &path);
    QUrl buildUrl(const QString &path);
    QString sendToServer(const QUrl &url, const QString &json, NetActionType netAction);

    bool m_connected;
    QString m_server;
    int m_port;
    QWebSocket *m_notificationSocket;
    QTimer m_findServerTimer;
    static API *m_instance;
    bool m_busy;
};

#endif // API_H
