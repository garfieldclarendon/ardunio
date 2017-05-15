#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include <QObject>
#include <QMap>
#include <QWebSocketServer>

#include "GlobalDefs.h"

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
    bool sendMessage(int serialNumber, const QJsonObject &obj);

signals:
    void newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);
    void pingSignal(const QByteArray &data);

public slots:
    void addController(int serialNumber);
    void removeController(int serialNumber);

    //Web Socket slots
    void onNewConnection(void);
    void connectionClosed(void);
    void processTextMessage(QString message);

protected slots:
    void sendMessageSlot(int serialNumber, const QString &data);

private:
    void sendControllerInfo(int serialNumber, QWebSocket *socket);
    void sendMultiControllerConfig(int serialNumber, QWebSocket *socket);
    void pongReply(quint64 length, const QByteArray &);
    void pingSlot(void);

    static ControllerManager *m_instance;

    QWebSocketServer *m_server;
    QTimer *m_pingTimer;
    QMap<int, ControllerEntry *> m_controllerMap;
    QMap<QWebSocket *, ControllerEntry *> m_socketMap;
    int m_transactionID;
};

#endif // CONTROLLERMANAGER_H
