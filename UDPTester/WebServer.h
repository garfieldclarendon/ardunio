#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>

class QWebSocketServer;

class WebServer : public QObject
{
    Q_OBJECT
public:
    explicit WebServer(quint16 port, QObject *parent = 0);

signals:

public slots:
    void onNewConnection(void);
    void processTextMessage(const QString message);
    void processBinaryMessage(const QByteArray message);
    void socketDisconnected(void);

private:
    QByteArray getTurnoutConfig(int controllerID);
    QByteArray getPanelConfig(int controllerID);
    QByteArray getPanelRouteConfig(int controllerID);
    QByteArray getSignalConfig(int controllerID);
    QByteArray getBlockConfig(int controllerID);

    QWebSocketServer *m_webServer;
};

#endif // WEBSERVER_H
