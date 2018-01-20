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

void WebServer::handleUdpMessage(NetActionType action, const QString &path, const QString &payload)
{
    QUrl url(path);
    UrlHandler *handler = getUrlHandler(url.path());
    if(handler)
    {
        qDebug("FOUND HANDLER!");

        APIRequest request(action, QUrl(path));
        request.setPayload(payload.toLatin1());

        handler->handleRequest(request);
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

bool WebServer::sendMessage(NetActionType actionType, const QString &uri, const QString &payload, QString &returnPayload)
{
    QNetworkAccessManager networkManager;

    qDebug(QString("WebServer::sendMessage to: %1").arg(uri).toLatin1());
    QNetworkReply *reply = NULL;
    QNetworkRequest request;
    request.setUrl(QUrl(uri));

    if(actionType == NetActionGet)
    {
        reply = networkManager.get(request);
    }
    else if(actionType == NetActionAdd)
    {
        reply = networkManager.post(request, payload.toLatin1());
    }
    else if(actionType == NetActionUpdate)
    {
        reply = networkManager.put(request, payload.toLatin1());
    }
    else if(actionType == NetActionDelete)
    {
        reply = networkManager.deleteResource(request);
    }

    bool ret = false;
    if(reply)
    {
        while(reply->isFinished() == false)
            QCoreApplication::processEvents();

        ret = reply->error() == QNetworkReply::NoError;
        if(ret == false)
            qDebug(reply->errorString().toLatin1());
        returnPayload = reply->readAll();
        qDebug(QString("WebServer::sendMessage - reply data: %1").arg(returnPayload).toLatin1());
    }

    return ret;
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
