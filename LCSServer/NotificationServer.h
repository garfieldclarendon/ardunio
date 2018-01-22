#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>

class QWebSocketServer;
class QJsonDocument;
class QJsonObject;

class NotificationServer : public QObject
{
    Q_OBJECT
public:
    explicit NotificationServer(QObject *parent = 0);
    void startServer(quint16 port);

    static NotificationServer *instance(void);

signals:
    void broadcastTextMessage(const QString &message);

public slots:
    void sendNotificationMessage(const QString &url, const QJsonObject &obj);

protected slots:
    void onNewConnection(void);
    void processTextMessage(const QString message);
    void processBinaryMessage(const QByteArray message);
    void socketDisconnected(void);

private:
    static NotificationServer *m_instance;
    QWebSocketServer *m_webServer;
};

#endif // WEBSERVER_H
