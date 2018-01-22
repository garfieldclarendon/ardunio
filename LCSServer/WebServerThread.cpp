#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QCoreApplication>

#include "WebServerThread.h"
#include "WebServer.h"
#include "APIEntity.h"

WebServerThread::WebServerThread(int socketDescriptor, QObject *parent)
    : QThread(parent), m_socketDescriptor(socketDescriptor)
{
}

WebServerThread::~WebServerThread()
{
    qDebug(QString("Connection Closed").toLatin1());
}

void WebServerThread::run(void)
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(m_socketDescriptor))
    {
//        emit error(tcpSocket.error());
        return;
    }
    qDebug(QString("Web Server Thread:  New Connection from: %1").arg(socket.peerAddress().toString()).toLatin1());
    socket.waitForReadyRead();

    QString line = QString(socket.readLine());
    qDebug(line.simplified().toLatin1().constData());

    QStringList tokens = line.split(QRegExp("[ \r\n][ \r\n]*"));

    int payloadSize = 0;
    QString payload;
    APIResponse response;

    while(line.length() > 0)
    {
        line = socket.readLine();
        if(line.startsWith("Content-Length:"))
        {
            int index = line.indexOf(":");
            payloadSize = line.mid(index+1).toInt();
        }
    }
    if(payloadSize > 0)
    {
        // wait for all of the data to arrive
        while(socket.size() < payloadSize)
        {
            if(socket.waitForReadyRead() == false)
                return;
        }
        payload = socket.readAll();
    }

    QString url;
    url = tokens.value(1);
    QUrl u(url);
    if (tokens.value(0) == "GET" && u.path().startsWith("/web", Qt::CaseInsensitive))
    {
        QString path = QCoreApplication::applicationDirPath();
        QDir::setCurrent(path);
        path += u.path();
        if(path.endsWith(QChar('/')))
            path.chop(1);
        if(path.endsWith("web", Qt::CaseInsensitive))
            path += "/index.html";
        path.replace("web", "Files");
        QString returnCode("200 OK");
        // PROCESS //
        QByteArray fileData;
        fileData = getFile(path);
        response.setReturnCode(returnCode);
        response.setPayload(fileData);
    }
    else
    {
        handleSocket(tokens.value(1), tokens.value(0), payload, response);
    }

    int size = response.getPayload().size();
    QString header = WebServer::createHeader(response.getReturnCode(), size, response.getContentType());
    socket.write(header.toLatin1());
    if(size > 0)
    {
        socket.write(response.getPayload());
    }
    socket.waitForBytesWritten();
    socket.flush();
    socket.close();
    sleep(1);
}

QByteArray WebServerThread::getFile(const QString &fileName)
{
    QByteArray fileData;
    QFile file(fileName);

    if(file.open(QIODevice::ReadOnly))
    {
        fileData = file.readAll();
        file.close();
    }

    return fileData;
}

void WebServerThread::handleSocket(const QString &path, const QString &actionText, const QString &payload, APIResponse &response)
{
    WebServer *server = qobject_cast<WebServer *>(parent());

    QUrl url(path);
    NetActionType actionType = NetActionGet;
    if(actionText == "PUT")
        actionType = NetActionUpdate;
    else if(actionText == "POST")
        actionType = NetActionAdd;
    else if(actionText == "DELETE")
        actionType = NetActionDelete;
    if(url.path().startsWith("/api/entity", Qt::CaseInsensitive))
    {
        APIResponse r;
        APIRequest request(actionType, url);
        request.setPayload(payload.toLatin1());

        APIEntity entity;
        entity.handleClient(request, &r);
        response = r;
    }
    else
    {
        UrlHandler *handler = server->getUrlHandler(url.path());
        if(handler)
        {
            qDebug("FOUND HANDLER!");

            APIRequest request(actionType, url);
            response = handler->handleRequest(request);
        }
        else
        {
            qDebug("INVALID URL.  SENDING OK");
        }
    }
}
