#ifndef API_H
#define API_H

#include <QObject>
#include <QWebSocket>

#include "GlobalDefs.h"

class API : public QObject
{
    Q_OBJECT
public:
    explicit API(const QString &server, const int port, QObject *parent = nullptr);
    ~API(void);

signals:
    void controllerChanged(int serialNumber, ControllerStatus status);

public slots:
    QString getControllerList(void);
    QString getDeviceList(void);

protected slots:
    void textMessageReceived(const QString &message);
    void notificationStateChanged(QAbstractSocket::SocketState state);
    void connectNotificationSocket(void);

private:
    void setupNotificationSocket(void);
    QUrl buildNotifcationUrl(const QString &path);
    QUrl buildUrl(const QString &path);
    QString sendToServer(const QUrl &url, const QString &json, NetActionType netAction);

    QString m_server;
    int m_port;
    QWebSocket *m_notificationSocket;
};

#endif // API_H
