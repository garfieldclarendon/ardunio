#include <QTcpSocket>

#include "TcpServer.h"
#include "UDPMessage.h"

TcpServer::TcpServer(quint16 port, QObject *parent) : QTcpServer(parent), m_currentSocket(NULL)
{
    listen(QHostAddress::Any, port);
}

void TcpServer::incomingConnection(int socket)
{
    if(m_currentSocket == NULL)
    {
        m_currentSocket = new QTcpSocket(this);
        connect(m_currentSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
        connect(m_currentSocket, SIGNAL(disconnected()), this, SLOT(discardClient()));
        m_currentSocket->setSocketDescriptor(socket);

        qDebug(QString("New Connection from: %1").arg(m_currentSocket->peerAddress().toString()).toLatin1().constData());
        this->pauseAccepting();
    }
    else
    {
        qDebug(QString("New Connection ignored").toLatin1().constData());
    }
}

void TcpServer::readClient()
{
    // This slot is called when the client sent data to the server
    while (m_currentSocket->size())
    {
        int available = m_currentSocket->bytesAvailable();
        int structSize = sizeof(MessageStruct);
        if(available >= structSize)
        {
            MessageStruct datagram;
            m_currentSocket->read((char *)&datagram, sizeof(MessageStruct));

            UDPMessage message(datagram);
            emit newMessage(message);
            QString str(QString("Message: %1, Controller: %2, Device: %3 Version: %4 byteValue1 %5 byteValue2 %6").arg(datagram.messageID).arg(datagram.controllerID).arg(datagram.deviceID).arg(datagram.messageVersion).arg(datagram.payload.payloadStruct.byteValue1).arg(datagram.payload.payloadStruct.byteValue2));
            emit newRawUDPMessage(str);
        }
        else
        {
            qDebug("HELP!  WRONG SIZE!!!!");
        }
    }
    qDebug("exiting readClient");
}

void TcpServer::discardClient()
{
    QTcpSocket* s = (QTcpSocket*)sender();
    if(s == m_currentSocket)
    {
        m_currentSocket = NULL;
        resumeAccepting();
    }

    qDebug(QString("Connection Closed").toLatin1().constData());
    s->deleteLater();
}
