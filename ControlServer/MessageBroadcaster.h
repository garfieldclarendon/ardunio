#ifndef MESSAGEBROADCASTER_H
#define MESSAGEBROADCASTER_H

#include <QObject>
#include <QHostAddress>

#include "UDPMessage.h"

class QUdpSocket;
class QTcpServer;

union IP4AddressUnion
{
    quint32 address32;
    quint8 bytes[4];
};

class MessageBroadcaster : public QObject
{
    Q_OBJECT
    explicit MessageBroadcaster(QObject *parent);

public:

    void broadcastMessage(const QString &data);
    static MessageBroadcaster *instance();

signals:
    void newMessage(const UDPMessage &message);
    void newRawUDPMessage(const QString &message);

public slots:
    void sendResetCommand(int serialNumber);
    void sendDownloadFirmware(int serialNumber);
    void sendMessage(int messageID, int serialNumber, quint8 byte1, quint8 byte2 = 0, quint8 byte3 = 0, quint8 byte4 = 0, quint8 byte5= 0);
    void sendUDPMessage(const UDPMessage &message);
    bool sendUDPMessage(const UDPMessage &message, const QString &address);

protected slots:
    void processPendingMessages(void);
    void processUdpBuffer(void);
    void sendMessageSlot(void);
    void sendHeartbeatSlot(const UDPMessage &message);
    void sendKeepAliveMessageSlot(void);
    void controllerRestarting(const UDPMessage &message);

private:
    void setupSocket(void);
    QHostAddress getLocalAddress(void) const;

    static MessageBroadcaster *_this;
    QUdpSocket *socket;
    qint64 lastMessageSentTime;
    QList<UDPMessage> sendList;
    quint16 m_udpPort;
    QByteArray m_udpBuffer;
};

#endif // MESSAGEBROADCASTER_H
