#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QUdpSocket>

#include "WebServer.h"
#include "Database.h"
#include "WebServerThread.h"
#include "MessageBroadcaster.h"

WebServer *WebServer::m_instance = NULL;
WebServer::WebServer(QObject *parent)
    : QTcpServer(parent)
{
}

void WebServer::incomingConnection(int socket)
{
    WebServerThread *client = new WebServerThread(socket, this);
    connect(client, SIGNAL(finished()), client, SLOT(deleteLater()));
    client->start();
}

void WebServer::startServer(quint16 port)
{
    qDebug(QObject::tr("WebServer::startServer on port: %1").arg(port).toLatin1());
    bool ret = listen(QHostAddress::Any, port);
    if(ret)
    {
        qDebug(QObject::tr("WebServer::startServer STARTED!").toLatin1());
    }
    else
    {
        qDebug(QObject::tr("WebServer::startServer FAILED!! %1").arg(this->errorString()).toLatin1());
    }
}

WebServer *WebServer::instance()
{
    if(m_instance == NULL)
        m_instance = new WebServer(qApp);
    return m_instance;
}


UrlHandler *WebServer::createUrlHandler(const QString &path)
{
    UrlHandler *handler;
    if(m_handlerMap.contains(path))
    {
        handler = m_handlerMap.value(path);
        handler->deleteLater();
    }
    handler = new UrlHandler(this);
    m_handlerMap[path] = handler;

    return handler;
}

UrlHandler *WebServer::getUrlHandler(const QString &path)
{
    return m_handlerMap.value(path);
}

QString WebServer::createHeader(const QString &httpCode, int bodySize, const QString &contentType)
{
    QString header;
    header = QString("HTTP/1.0 %1\r\n"
            "Content-Type: %2;\r\n").arg(httpCode).arg(contentType);
    if(bodySize > 0)
            header += QString("Content-Length: %1\r\n").arg(bodySize);
    header += "\r\n";

    return header;
}
