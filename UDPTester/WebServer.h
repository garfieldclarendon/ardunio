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
    QByteArray getTurnoutConfig(quint32 serialNumber, int moduleIndex);
    QByteArray getPanelConfig(quint32 serialNumber);
    QByteArray getPanelRouteConfig(quint32 serialNumber);
    QByteArray getSignalConfig(quint32 serialNumber, int moduleIndex);
    QByteArray getBlockConfig(quint32 serialNumber, int moduleIndex);
    QByteArray getMultiControllerConfig(quint32 serialNumber, quint32 moduleIndex);

    QWebSocketServer *m_webServer;
};

#endif // WEBSERVER_H
