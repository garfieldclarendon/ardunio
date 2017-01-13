#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QApplication>
#include <QDateTime>
#include <QTimer>
#include <QNetworkInterface>

#include "MessageBroadcaster.h"
#include "TcpClientHandler.h"
#include "GlobalDefs.h"

MessageBroadcaster *MessageBroadcaster::_this = NULL;

MessageBroadcaster::MessageBroadcaster(QObject *parent)
    : QObject(parent), socket(NULL), tcpServer(NULL), lastMessageSentTime(0), m_sendHeartbeat(false), m_udpPort(45454)
{
    setupSocket();
}

void MessageBroadcaster::broadcastMessage(const QString &data)
{
    UDPMessage message(data);

    emit newMessage(message);
}

MessageBroadcaster *MessageBroadcaster::instance()
{
    if(_this == NULL)
        _this = new MessageBroadcaster(qApp);
    return _this;
}

void MessageBroadcaster::setupSocket()
{
    socket = new QUdpSocket(this);
    socket->bind(m_udpPort, QUdpSocket::ShareAddress);

    connect(socket, SIGNAL(readyRead()),
            this, SLOT(processPendingMessages()));

    tcpServer = new QTcpServer(this);
    if(tcpServer->listen(QHostAddress::Any, m_udpPort))
        qDebug(QString("TCP SERVER IS LISTENING ON PORT %1").arg(tcpServer->serverPort()).toLatin1());
    else
        qDebug(QString("ERROR STARTING TCP SERVER!!!!  %1").arg(tcpServer->errorString()).toLatin1());
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(tcpIncomingConnection()));
}

QHostAddress MessageBroadcaster::getLocalAddress() const
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    QHostAddress address;

    for(int nIter=0; nIter<list.count(); nIter++)
    {
        if(!list[nIter].isLoopback())
        {
            if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol )
            {
                address = list[nIter];
            }
        }
    }

    return address;
}

void MessageBroadcaster::sendUDPMessage(const UDPMessage &message)
{
    if(QDateTime::currentDateTime().toMSecsSinceEpoch() - lastMessageSentTime > sendTimeout)
    {
        lastMessageSentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        int sent = socket->writeDatagram(message.getMessageRef(), sizeof(MessageStruct),
                                 QHostAddress::Broadcast, m_udpPort);
        qDebug(QString("sendUDPMessage: Message Sent, size: %1").arg(sent).toLatin1());
    }
    else
    {
        qDebug(QString("sendUDPMessage: Buffering messages.  Adding message to send queue").toLatin1());
        sendList << message;
        QTimer::singleShot(sendTimeout, this, SLOT(sendMessageSlot()));
    }
}

void MessageBroadcaster::processPendingMessages()
{
    while (socket->hasPendingDatagrams())
    {
        int available = socket->bytesAvailable();
        int structSize = sizeof(MessageStruct);
        if(available >= structSize)
        {
            MessageStruct datagram;
            socket->readDatagram((char *)&datagram, sizeof(MessageStruct));

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
}

void MessageBroadcaster::tcpIncomingConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    TcpClientHandler *handler = new TcpClientHandler(clientConnection);

    connect(clientConnection, SIGNAL(disconnected()),
            handler, SLOT(deleteLater()));
}

void MessageBroadcaster::sendMessageSlot()
{
    UDPMessage message = sendList.takeAt(0);
    sendUDPMessage(message);
    if(sendList.count() > 0)
        QTimer::singleShot(sendTimeout, this, SLOT(sendMessageSlot()));
}

void MessageBroadcaster::enableHeartbeat(bool enable)
{
    m_sendHeartbeat = enable;
    if(enable)
    {
        QTimer::singleShot(100, this, SLOT(sendHeartbeatSlot()));
    }
}

void MessageBroadcaster::sendHeartbeatSlot()
{
    if(m_sendHeartbeat)
    {
        UDPMessage message;
        message.setMessageID(SYS_HEARTBEAT);
        message.setMessageClass(ClassSystem);
        QHostAddress address = getLocalAddress();
        IP4AddressUnion a;
        a.address32 = address.toIPv4Address();
        message.setField(0, a.bytes[3]);
        message.setField(1, a.bytes[2]);
        message.setField(2, a.bytes[1]);
        message.setField(3, a.bytes[0]);
        message.setField(4, 81);

        sendUDPMessage(message);

        QTimer::singleShot(5000, this, SLOT(sendHeartbeatSlot()));
    }
}

void MessageBroadcaster::sendConfigData(int controllerID)
{
    UDPMessage message;
    message.setMessageID(SYS_CONFIG_CHANGED);
    message.setControllerID(controllerID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendResetCommand(int controllerID)
{
    UDPMessage message;
    message.setMessageID(SYS_REBOOT_CONTROLLER);
    message.setControllerID(controllerID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendDownloadFirmware(int controllerID)
{
    UDPMessage message;
    message.setMessageID(SYS_DOWNLOAD_FIRMWARE);
    message.setControllerID(controllerID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendMessage(int messageID, int controllerID, int deviceID, int messageClass, long lValue, int intValue1, int intValue2, int byteValue1, int byteValue2)
{
    UDPMessage message;
    message.setMessageID(messageID);
    message.setControllerID(controllerID);
    message.setDeviceID(deviceID);
    message.setMessageClass(messageClass);
    message.setLValue(lValue);
    message.setIntValue1(intValue1);
    message.setIntValue2(intValue2);
    message.setByteValue1(byteValue1);
    message.setByteValue2(byteValue2);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}
