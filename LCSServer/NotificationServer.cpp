#include <QWebSocketServer>
#include <QWebSocket>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>

#include "NotificationServer.h"
#include "Database.h"

NotificationServer * NotificationServer::m_instance = NULL;

NotificationServer::NotificationServer(QObject *parent) : QObject(parent),
 m_webServer(new QWebSocketServer(QStringLiteral("Config Server"), QWebSocketServer::NonSecureMode, this))
{
}

NotificationServer *NotificationServer::instance()
{
    if(m_instance == NULL)
        m_instance = new NotificationServer(qApp);
    return m_instance;
}
void NotificationServer::startServer(quint16 port)
{
    qDebug() << "NotificationServer Starting on port " << port;
    if (m_webServer->listen(QHostAddress::Any, port))
    {
        qDebug() << "NotificationServer Started!!";
        connect(m_webServer, &QWebSocketServer::newConnection,
                this, &NotificationServer::onNewConnection);
    }
    else
    {
        qDebug() << "NotificationServer FAILED TO START!!!";
    }
}


void NotificationServer::sendNotificationMessage(const QString &url, const QJsonObject &obj)
{
    QJsonObject o(obj);
    o["url"] = url;
    QJsonDocument doc;
    doc.setObject(o);

    emit broadcastTextMessage(doc.toJson());
}

void NotificationServer::onNewConnection()
{
    QWebSocket *pSocket = m_webServer->nextPendingConnection();

    connect(this, &NotificationServer::broadcastTextMessage, pSocket, &QWebSocket::sendTextMessage);
    connect(pSocket, &QWebSocket::textMessageReceived, this, &NotificationServer::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &NotificationServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &NotificationServer::socketDisconnected);
}

void NotificationServer::processTextMessage(const QString message)
{
    Q_UNUSED(message);
}

void NotificationServer::processBinaryMessage(const QByteArray message)
{
    Q_UNUSED(message);
}

void NotificationServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "socketDisconnected:" << pClient;
    if (pClient)
    {
        pClient->deleteLater();
    }
}
