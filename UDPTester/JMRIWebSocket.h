#ifndef JMRIWEBSOCKET_H
#define JMRIWEBSOCKET_H

#include <QObject>
#include <QUrl>
#include <QAbstractSocket>
#include <QTimer>

#include "UDPMessage.h"
#include "GlobalDefs.h"

class QWebSocket;

class JMRIWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit JMRIWebSocket(const QUrl &url, QObject *parent = 0);

signals:
    void newMessage(const UDPMessage &message);

public slots:
    void onNewMessage(const UDPMessage &message);

    void onConnected(void);
    void processTextMessage(const QString message);
    void processBinaryMessage(const QByteArray message);
    void closed(void);
    void onError(QAbstractSocket::SocketError error);
    void heatbeatTimerProc(void);

    void testPostReadyRead(void);

private:
    void connectToJMRI(void);
    void sendTurnoutStatusMessage(const QString &name, TurnoutState currentState);
    void testPost(void);

    QWebSocket *m_webSocket;
    QUrl m_url;
    bool m_ignoreMessage;
    QMap<int, int> m_turnoutMap;
    QTimer m_heartbeatTimer;
};

#endif // JMRIWEBSOCKET_H
