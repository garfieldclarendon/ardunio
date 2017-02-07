#include <QWebSocketServer>
#include <QWebSocket>
#include <QStringList>
#include "WebServer.h"
#include "Database.h"

WebServer::WebServer(quint16 port, QObject *parent) : QObject(parent),
 m_webServer(new QWebSocketServer(QStringLiteral("Config Server"), QWebSocketServer::NonSecureMode, this))
{
    qDebug() << "WebServer Starting";
    if (m_webServer->listen(QHostAddress::Any, port))
    {
        qDebug() << "WebServer Started!!";
        connect(m_webServer, &QWebSocketServer::newConnection,
                this, &WebServer::onNewConnection);
    }
}

void WebServer::onNewConnection()
{
    QWebSocket *pSocket = m_webServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &WebServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WebServer::socketDisconnected);
}

void WebServer::processTextMessage(const QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Message received:" << message;
    QStringList parts = message.split(',');
    QByteArray configData;
    if(parts[1] == "T")
    {
        configData = getTurnoutConfig(parts[0].toInt(), parts.value(2).toInt());
    }
    else if(parts[1] == "S")
    {
        configData = getSignalConfig(parts[0].toInt(), parts.value(2).toInt());
    }
    else if(parts[1] == "B")
    {
        configData = getBlockConfig(parts[0].toInt());
    }
    else if(parts[1] == "P")
    {
        if(parts[2] == "1")
            configData = getPanelConfig(parts[0].toInt());
        else
            configData = getPanelRouteConfig(parts[0].toInt());
    }
    if (pClient)
    {
        pClient->sendBinaryMessage(configData);
        pClient->close();
    }
}

void WebServer::processBinaryMessage(const QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Binary Message received:" << message;
    if (pClient)
    {
        pClient->sendBinaryMessage(message);
    }
}

void WebServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << pClient;
    if (pClient)
    {
        pClient->deleteLater();
    }
}

QByteArray WebServer::getTurnoutConfig(quint32 serialNumber, int moduleIndex)
{
    qDebug() << "getTurnoutConfig: " << serialNumber;
    QByteArray data;

    Database db;
    data = db.getTurnoutConfig(serialNumber, moduleIndex);

    return data;
}

QByteArray WebServer::getPanelConfig(quint32 serialNumber)
{
    QByteArray data;

    Database db;
    data = db.getPanelConfig(serialNumber);

    return data;
}

QByteArray WebServer::getPanelRouteConfig(quint32 serialNumber)
{
    QByteArray data;

    Database db;
    data = db.getPanelRouteConfig(serialNumber);

    return data;
}

QByteArray WebServer::getSignalConfig(quint32 serialNumber, int moduleIndex)
{
    QByteArray data;

    Database db;
    data = db.getSignalConfig(serialNumber, moduleIndex);

    return data;
}

QByteArray WebServer::getBlockConfig(quint32 serialNumber)
{
    QByteArray data;

    Database db;
    data = db.getBlockConfig(serialNumber);

    return data;
}
