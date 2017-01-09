#include <QTcpSocket>
#include <QUrl>
#include <QtCore/QUrlQuery>
#include <QFile>

#include "UpdateServer.h"

UpdateServer::UpdateServer(quint16 port, QObject *parent) : QTcpServer(parent), m_contentLength(0)
{
    listen(QHostAddress::Any, port);
}

void UpdateServer::incomingConnection(int socket)
{
    // When a new client connects, the server constructs a QTcpSocket and all
    // communication with the client is done over this QTcpSocket. QTcpSocket
    // works asynchronously, this means that all the communication is done
    // in the two slots readClient() and discardClient().
    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(socket);

    qDebug(QString("New Connection from: %1").arg(s->peerAddress().toString()).toLatin1().constData());
}

void UpdateServer::readClient()
{
    // This slot is called when the client sent data to the server
    QTcpSocket* socket = (QTcpSocket*)sender();
    if(m_contentLength == 0 && socket->canReadLine())
    {
        QString line = QString(socket->readLine());
        qDebug(line.simplified().toLatin1().constData());

        QStringList tokens = line.split(QRegExp("[ \r\n][ \r\n]*"));
        if (tokens.value(0) == "GET")
        {
            // GATHER DATA //
            QString url;
            url = tokens.value(1);
            QUrl u(url);
            QUrlQuery query(u);
            QString controllerType(query.queryItemValue("ControllerType"));

            QString fileName;
            QString path;
//            path = "C:/Users/John/Documents/Arduino/";
            path = "C:/Users/Jreil/OneDrive/Documents/Arduino/";
            QString returnCode("200 OK");
            if(controllerType == "1")
                fileName = path + "TurnoutController/Release/TurnoutController.ino.bin";
            else if(controllerType == "2")
                fileName = path + "PanelController/Release/PanelController.ino.bin";
            else if(controllerType == "4")
                fileName = path + "SignalController/Release/SignalController.ino.bin";
            else if(controllerType == "5")
                fileName = path + "SemaphoreController/Release/SemaphoreController.ino.bin";

            // PROCESS //
            QByteArray fileData;
            if(fileName.length() > 0)
                fileData = getFile(fileName);
            QString result = createHeader(returnCode, fileData.size());

            socket->write(result.toLatin1());

            socket->write(fileData);

            socket->flush();
            socket->close();
        }
    }
    qDebug("exiting readClient");
}

void UpdateServer::discardClient()
{
    QTcpSocket* s = (QTcpSocket*)sender();
    qDebug(QString("Connection Closed").toLatin1().constData());
    s->deleteLater();
}

QString UpdateServer::createHeader(const QString &httpCode, int bodySize)
{
    QString header;
    header = QString("HTTP/1.1 %1\r\n"
            "Content-Type: application/octet-stream;\r\n"
            "Content-Length: %2\r\n"
            "\r\n").arg(httpCode).arg(bodySize);

    return header;
}

QByteArray UpdateServer::getFile(const QString &fileName)
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
