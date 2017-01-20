#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class UDPMessage;

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(quint16 port, QObject *parent = 0);

    void incomingConnection(int socket);

signals:
    void newMessage(const UDPMessage &message);
    void newRawUDPMessage(const QString &message);

public slots:
    void readClient(void);
    void discardClient(void);

private:
    QTcpSocket *m_currentSocket;
};

#endif // TCPSERVER_H
