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
    explicit MessageBroadcaster(QObject *parent = 0);

public:

    void broadcastMessage(const QString &data);
    static MessageBroadcaster *instance(void);

signals:
    void newMessage(const UDPMessage &message);
    void newRawUDPMessage(const QString &message);

public slots:
    void enableHeartbeat(bool enable);
    void sendConfigData(int controllerID);
    void sendResetCommand(int controllerID);
    void sendDownloadFirmware(int controllerID);
    void sendMessage(int messageID, int controllerID, int deviceID, int messageClass, long lValue, int intValue1, int intValue2, int byteValue1, int byteValue2);
    void sendUDPMessage(const UDPMessage &message);

protected slots:
    void processPendingMessages();
    void tcpIncomingConnection(void);
    void sendMessageSlot(void);
    void sendHeartbeatSlot(void);

private:
    void setupSocket(void);
    QHostAddress getLocalAddress(void) const;

    static MessageBroadcaster *_this;
    QUdpSocket *socket;
    QTcpServer *tcpServer;
    qint64 lastMessageSentTime;
    QList<UDPMessage> sendList;
    bool m_sendHeartbeat;
    quint16 m_udpPort;
};

#endif // MESSAGEBROADCASTER_H
