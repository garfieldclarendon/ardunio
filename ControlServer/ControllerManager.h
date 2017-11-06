#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include <QObject>
#include <QMap>
#include <QWebSocketServer>

#include "GlobalDefs.h"
#include "controllermessage.h"

class ControllerEntry;
class QWebSocket;
class QJsonObject;
class QTimer;

class ControllerManager : public QObject
{
    Q_OBJECT
    explicit ControllerManager(QObject *parent = 0);
public:
    ~ControllerManager(void);

    static ControllerManager *instance(void);
    QString getControllerIPAddress(int serialNumber) const;
    bool sendMessage(const ControllerMessage &message);
    int getConnectionCount(void) const { return m_socketList.count(); }
    int getConnectionSerialNumber(int index) const;
    void getConnectedInfo(int serialNumber, int &version, ControllerStatus &status);

signals:
    void newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);
    void pingSignal(const QByteArray &data);

    void controllerConnected(int index);
    void controllerDisconnected(int index);
    void controllerAdded(int serialNumber);
    void controllerRemoved(int serialNumber);
    void controllerPing(int serialNumber, quint64 length);
    void sendNotificationMessage(const QString &uri, QJsonObject &obj);

    void messageACKed(const ControllerMessage &message);
    void errorSendingMessage(const ControllerMessage &message);

public slots:
    //Web Socket slots
    void onNewConnection(void);
    void connectionClosed(void);
    void processTextMessage(QString message);
    void controllerResetting(long serialNumber);

protected slots:
    void sendMessageSlot(int transactionID, int serialNumber, const QString &data);

private:
    void sendControllerInfo(int serialNumber, QWebSocket *socket);
    void sendMultiControllerConfig(int serialNumber, QWebSocket *socket);
    void sendPanelControllerConfig(int serialNumber, QWebSocket *socket);
    void pongReply(quint64 length, const QByteArray &);
    void pingSlot(void);
    void createAndSendNotificationMessage(int serialNumber, ControllerStatus status, quint64 pingLength = -1);

    static ControllerManager *m_instance;

    QWebSocketServer *m_server;
    QTimer *m_pingTimer;
    QList<QWebSocket *> m_socketList;
    QMap<int, ControllerMessage> m_messageMap;
};

#endif // CONTROLLERMANAGER_H
