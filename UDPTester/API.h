#ifndef API_H
#define API_H

#include <QObject>
#include <QWebSocket>
#include <QUdpSocket>

#include "GlobalDefs.h"

class UDPMessage;

class API : public QObject
{
    Q_OBJECT
public:
    explicit API(const QString &server, const int port, QObject *parent = nullptr);
    explicit API(QObject *parent = nullptr);
    ~API(void);

    static API *instance(void);

signals:
    void controllerChanged(int serialNumber, ControllerStatus status);
    void deviceChanged(int deviceID, int status);
    void apiReady(void);

public slots:
    QString getControllerList(void);
    QString getDeviceList(void);

    bool getApiReady(void) { return m_server.length() > 0; }

protected slots:
    void textMessageReceived(const QString &message);
    void notificationStateChanged(QAbstractSocket::SocketState state);
    void connectNotificationSocket(void);
    void newUDPMessage(const UDPMessage &message);

private:
    void setupNotificationSocket(void);
    void setupUDPSocket(void);
    QUrl buildNotifcationUrl(const QString &path);
    QUrl buildUrl(const QString &path);
    QString sendToServer(const QUrl &url, const QString &json, NetActionType netAction);

    QString m_server;
    int m_port;
    QWebSocket *m_notificationSocket;
    static API *m_instance;
};

#endif // API_H
