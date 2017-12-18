#include <QUdpSocket>
#include <QDateTime>
#include <QTimer>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QCoreApplication>
#include <QTcpSocket>

#include "MessageBroadcaster.h"
#include "GlobalDefs.h"

MessageBroadcaster *MessageBroadcaster::_this = NULL;
bool MessageBroadcaster::m_runAsClient = false;

MessageBroadcaster::MessageBroadcaster(QObject *parent)
    : QObject(parent), socket(NULL), lastMessageSentTime(0), m_udpPort(UdpPort)
{
    setupSocket();
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
    if(socket->bind(m_udpPort, QUdpSocket::ShareAddress))
        qDebug(QString("UDP CLIENT IS LISTENING ON PORT %1").arg(m_udpPort).toLatin1());
    else
        qDebug(QString("ERROR STARTING UDP CLIENT!!!!  %1").arg(socket->errorString()).toLatin1());

    connect(socket, SIGNAL(readyRead()),
            this, SLOT(processPendingMessages()));
    if(!m_runAsClient)
    {
        sendHeartbeatSlot(UDPMessage());
        QTimer::singleShot(HEARTBEAT_INTERVAL, this, SLOT(heartbeatTimerSlot()));
    }
//    QTimer::singleShot(60000, this, SLOT(sendKeepAliveMessageSlot()));
}

QHostAddress MessageBroadcaster::getLocalAddress() const
{
    QHostAddress returnAddress;
    QTcpSocket dnsTestSocket;

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
    {
        QString guessedGatewayAddress = address.toString().section( ".",0,2 ) + ".1";

        if (address.protocol() == QAbstractSocket::IPv4Protocol
                && address != QHostAddress(QHostAddress::LocalHost))
        {
            dnsTestSocket.connectToHost(guessedGatewayAddress, 53);
            if (dnsTestSocket.waitForConnected(3000))
            {
                returnAddress = dnsTestSocket.localAddress();
                break;
            }
            if(address.toString().startsWith("192"))
                returnAddress = address;
        }
    }

    return returnAddress;
}

void MessageBroadcaster::sendUDPMessage(const UDPMessage &message)
{
    int sent = socket->writeDatagram(message.getMessageRef(), sizeof(MessageStruct),
                             QHostAddress::Broadcast, m_udpPort);
    qDebug(QString("sendUDPMessage: Message Sent, size: %1").arg(sent).toLatin1());
}

bool MessageBroadcaster::sendUDPMessage(const UDPMessage &message, const QString &address)
{
    bool ret = true;
    int count;
    if((count = socket->writeDatagram(message.getMessageRef(), sizeof(MessageStruct), QHostAddress(address), m_udpPort)) < 0)
        ret = false;
    else
        ret = true;

    qDebug(QString("MessageBroadcaster::sendUdpMessage: address: %1 return %3").arg(address).arg(count).toLatin1());

    return ret;
}

void MessageBroadcaster::processPendingMessages()
{
    int available = socket->pendingDatagramSize();
    QByteArray buffer;
    buffer.resize(available);
    QHostAddress address;
    socket->readDatagram(buffer.data(), available, &address);
    m_udpBuffer.append(buffer);

    processUdpBuffer(address);
}

void MessageBroadcaster::processUdpBuffer(const QHostAddress &address)
{
    qDebug("processUdpBuffer");
    static bool signitureFound = false;
    int structSize = sizeof(MessageStruct);

    while(m_udpBuffer.size() >= structSize)
    {
        unsigned char byte1(0);
        unsigned char byte2(0);
        // Find the start of a valid message.
        // ...ignore everything else
        int startIndex = 0;
        if (signitureFound == false)
        {
            byte1 = m_udpBuffer[startIndex++];
            byte2 = m_udpBuffer[startIndex++];
            while (byte1 != 0xEE && byte2 != 0xEF && startIndex < m_udpBuffer.size())
            {
                byte1 = m_udpBuffer[startIndex++];
                byte2 = m_udpBuffer[startIndex++];
            }
            if (byte1 == 0xEE  && byte2 == 0xEF)
            {
                signitureFound = true;
            }
        }
        if(signitureFound)
        {
            unsigned int size = sizeof(MessageStruct);
            MessageStruct datagram;
            memset(&datagram, 0, sizeof(MessageStruct));
            char *buffer = (char *)&datagram;
            *buffer = byte1;
            buffer++;
            *buffer = byte2;
            buffer++;
            unsigned char data, nextByte;
            while(startIndex < m_udpBuffer.size())
            {
                data = m_udpBuffer[startIndex];
                nextByte = m_udpBuffer[++startIndex];
                if(size <= sizeof(MessageStruct))
                {
                    *buffer = data;
                    size--;
                    buffer++;
                }
                if (data == 0xEF && nextByte == 0xEE) // found end of message signature
                {
                    data = m_udpBuffer[startIndex += 2];
                    // Now remove any extra data
                    while (data != 0xEE && startIndex < m_udpBuffer.size())
                    {
                        data = m_udpBuffer[startIndex++];
                    }
                    if(data == 0xEE)
                        startIndex--;
                    break;
                }
            }

            m_udpBuffer.remove(0, startIndex);

            UDPMessage message(datagram);
            if(message.getMessageID() == SYS_ACK)
            {
                // ignore the ack for now
                QString str(QString("GOT ACK MESSAGE FROM %1 FOR: %2").arg(datagram.serialNumber).arg(datagram.transactionNumber));
                qDebug(str.toLatin1());
            }
            else
            {
                if(!address.isNull() && !address.isLoopback() && address != QHostAddress::Broadcast)
                {
                    UDPMessage ack;
                    ack.setMessageID(SYS_ACK);
                    ack.setTransactionNumber(message.getTransactionNumber());
                    qDebug(QString("SEND ACK MESSAGE TO %1 Transaction %2").arg(address.toString()).arg(ack.getTransactionNumber()).toLatin1());
                    socket->writeDatagram(ack.getMessageRef(), sizeof(MessageStruct), address, m_udpPort);
                }

                emit newMessage(message);
                QString str(QString("Message: %1, Controller: %2 Transaction: %3 byteValue1 %4 byteValue2 %5").arg(datagram.messageID).arg(datagram.serialNumber).arg(datagram.transactionNumber).arg(datagram.payload[0]).arg(datagram.payload[1]));
                qDebug(str.toLatin1());
                emit newRawUDPMessage(str);
                if(message.getMessageID() == SYS_CONTROLLER_ONLINE && !m_runAsClient)
                    sendHeartbeatSlot(message);
                else if(message.getMessageID() == SYS_RESTARTING)
                    controllerRestarting(message);
            }

            signitureFound = false;
        }
    }
}

void MessageBroadcaster::sendHeartbeatSlot(const UDPMessage &)
{
    static quint8 firstTime = 1;
    UDPMessage outMessage;
    outMessage.setMessageID(SYS_SERVER_HEARTBEAT);
    QHostAddress address = getLocalAddress();
    IP4AddressUnion a;
    a.address32 = address.toIPv4Address();
    outMessage.setField(0, a.bytes[3]);
    outMessage.setField(1, a.bytes[2]);
    outMessage.setField(2, a.bytes[1]);
    outMessage.setField(3, a.bytes[0]);
    outMessage.setField(4, firstTime);
    firstTime = 0;

    sendUDPMessage(outMessage);
}

void MessageBroadcaster::sendResetNotificationListCommand(int serialNumber)
{
    UDPMessage outMessage;
    outMessage.setMessageID(SYS_RESET_NOTIFICATION_LIST);
    outMessage.setSerialNumber(serialNumber);

    sendUDPMessage(outMessage);
}

void MessageBroadcaster::heartbeatTimerSlot()
{
    UDPMessage message;
    sendHeartbeatSlot(message);
    QTimer::singleShot(HEARTBEAT_INTERVAL, this, SLOT(heartbeatTimerSlot()));
}

void MessageBroadcaster::sendKeepAliveMessageSlot()
{
    UDPMessage outMessage;
    outMessage.setMessageID(SYS_KEEP_ALIVE);

    sendUDPMessage(outMessage);
    QTimer::singleShot(60000, this, SLOT(sendKeepAliveMessageSlot()));
}

void MessageBroadcaster::controllerRestarting(const UDPMessage &message)
{
    emit controllerResetting(message.getSerialNumber());
}

void MessageBroadcaster::sendResetCommand(int serialNumber)
{
    UDPMessage message;
    message.setMessageID(SYS_REBOOT_CONTROLLER);
    message.setSerialNumber(serialNumber);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendResetConfigCommand(int serialNumber)
{
    UDPMessage message;
    message.setMessageID(SYS_RESET_CONFIG);
    message.setSerialNumber(serialNumber);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendResetDeviceConfigCommand(int deviceID)
{
    UDPMessage message;
    message.setMessageID(SYS_RESET_DEVICE_CONFIG);
    message.setSerialNumber(deviceID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendLockRouteCommand(int routeID, bool lock)
{
    UDPMessage message;
    message.setMessageID(SYS_LOCK_ROUTE);
    message.setSerialNumber(routeID);
    message.setField(0, lock);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendDownloadFirmware(int serialNumber)
{
    UDPMessage message;
    message.setMessageID(SYS_DOWNLOAD_FIRMWARE);
    message.setSerialNumber(serialNumber);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MessageBroadcaster::sendMessage(int messageID, int serialNumber, quint8 byte1, quint8 byte2, quint8 byte3, quint8 byte4, quint8 byte5)
{
    UDPMessage message;
    message.setMessageID(messageID);
    message.setSerialNumber(serialNumber);
    message.setField(0, byte1);
    message.setField(1, byte2);
    message.setField(2, byte3);
    message.setField(3, byte4);
    message.setField(4, byte5);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}
