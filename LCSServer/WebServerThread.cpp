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

WebServerThread::WebServerThread(int socketDescriptor, QObject *parent)
    : QThread(parent), socketDescriptor(socketDescriptor), m_apiHandler(this)
{
}

WebServerThread::~WebServerThread()
{
    qDebug(QString("Connection Closed").toLatin1());
}

void WebServerThread::run(void)
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(socketDescriptor))
    {
//        emit error(tcpSocket.error());
        return;
    }
    qDebug(QString("Web Server Thread:  New Connection from: %1").arg(socket.peerAddress().toString()).toLatin1().constData());
    socket.waitForReadyRead();

    QString line = QString(socket.readLine());
    qDebug(line.simplified().toLatin1().constData());

    QStringList tokens = line.split(QRegExp("[ \r\n][ \r\n]*"));

    int payloadSize = 0;
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
        QString payload = socket.readAll();
        handleSocket(&socket, tokens.value(1), tokens.value(0).toUpper(), payload);
    }

    if(payloadSize == 0)
    {
        QString url;
        url = tokens.value(1);
        QUrl u(url);
        if (tokens.value(0) == "GET" && u.path() == "/controller/firmware")
        {
            handleDownloadFirmware(&socket, u);
            socket.close();
        }
        else if (tokens.value(0) == "GET" && u.path().startsWith("/web", Qt::CaseInsensitive))
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
            QString result = WebServer::createHeader(returnCode, fileData.size());
            socket.write(result.toLatin1());
            socket.write(fileData);
            socket.flush();
        }
        else
        {
            handleSocket(&socket, tokens.value(1), tokens.value(0), QString());
        }
    }
}

void WebServerThread::readClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(socket->property("contentLength").toInt() == 0 && socket->canReadLine())
    {
        QString line = QString(socket->readLine());
        qDebug(line.simplified().toLatin1().constData());

        QStringList tokens = line.split(QRegExp("[ \r\n][ \r\n]*"));

        int payloadSize = 0;
        while(line.length() > 0)
        {
            line = socket->readLine();
            if(line.startsWith("Content-Length:"))
            {
                int index = line.indexOf(":");
                payloadSize = line.mid(index+1).toInt();
                socket->setProperty("contentLength", payloadSize);
            }
        }
        socket->setProperty("actionType", tokens.value(0));
        socket->setProperty("path", tokens.value(1));
        if(payloadSize == 0)
        {
            QString url;
            url = tokens.value(1);
            QUrl u(url);
            if (tokens.value(0) == "GET" && u.path() == "/firmware")
            {
                handleDownloadFirmware(socket, u);
                socket->close();
            }
            else
            {
                handleSocket(socket, tokens.value(1), tokens.value(0), QString());
            }
        }
    }
    else if(socket->property("contentLength").toInt() <= socket->size())
    {
        QString payload = socket->readAll();
        handleSocket(socket, socket->property("path").toString(), socket->property("actionType").toString(), payload);
    }
    qDebug("exiting readClient");
}

void WebServerThread::handleDownloadFirmware(QTcpSocket* socket, const QUrl &url)
{
    qDebug("DOWNLOAD FIRMAWARE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    QUrlQuery query(url);
    QString controllerType(query.queryItemValue("ControllerType"));

    QString fileName;
    QString path = QCoreApplication::applicationDirPath();
    QDir::setCurrent(path);
    path += "/Builds/";
    QString returnCode("200 OK");
    fileName = path + "LCSController.ino.bin";

    qDebug(fileName.toLatin1());
    // PROCESS //
    QByteArray fileData;
    if(fileName.length() > 0)
        fileData = getFile(fileName);
    QString result = WebServer::createHeader(returnCode, fileData.size(), "application/octet-stream");

    socket->write(result.toLatin1());

    socket->write(fileData);

    socket->flush();
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

void WebServerThread::handleSocket(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload)
{
    WebServer *server = qobject_cast<WebServer *>(parent());

    QPointer<QTcpSocket> pSocket(socket);
    QUrl url(path);
    UrlHandler *handler = server->getUrlHandler(url.path());
    if(url.path().startsWith("/api/", Qt::CaseInsensitive))
    {
        m_apiHandler.handleClient(socket, path, actionText, payload);
    }
    else if(handler)
    {
        qDebug("FOUND HANDLER!");
        NetActionType actionType = NetActionGet;
        if(actionText == "PUT")
            actionType = NetActionUpdate;
        else if(actionText == "POST")
            actionType = NetActionAdd;
        else if(actionText == "DELETE")
            actionType = NetActionDelete;

        if(actionType != NetActionGet)
        {
            QString header = WebServer::createHeader("200 OK", 0);
            pSocket->write(header.toLatin1());
            pSocket->flush();
            pSocket->close();
            pSocket = NULL;
        }

        QString returnPayload = handler->handleRequest(actionType, url, payload);
        if(pSocket)
        {
            QString header = WebServer::createHeader("200 OK", returnPayload.length());
            pSocket->write(header.toLatin1());
            if(returnPayload.length() > 0)
            {
                pSocket->write(returnPayload.toLatin1());
            }
        }
    }
    else
    {
        QString header = WebServer::createHeader("200 OK", 0);

        qDebug("SEND OK");
        if(pSocket)
            pSocket->write(header.toLatin1());
    }
    if(pSocket)
    {
        pSocket->waitForBytesWritten();
        pSocket->flush();
        pSocket->close();
    }
}
