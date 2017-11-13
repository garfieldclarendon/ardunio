#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include <QObject>
#include <QMap>
#include <QWebSocketServer>

#include "GlobalDefs.h"
#include "controllermessage.h"


/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation.
// Device specific messages/notifcations are documented in their respective files
/**
 * @api {get} /controller/connect/:serialNumber,version Controller Connect Message
 * @apiName ControllerConnect
 * @apiGroup Controller
 *
 * @apiParam {Number} serialNumber The controller's serial number (chip ID).
 * @apiParam {Number} version The controller's firmware version number.
 * @apiDescription Sent from a controller once a web socket connection is made.
 */

/**
 * @api {get} /controller/multiConfig/:serialNumber Get configuration information for a specific MultiModule controller.
 * @apiName MultiModuleConfig
 * @apiGroup Controller
 *
 * @apiParam {Number} serialNumber The controller's serial number (chip ID).
 * @apiDescription Returns the coniguration information for a specific MultiModule controller.  This includes the controller classification and a list of modules, the module's classification and the module's address.
 * @apiSuccessExample {json} Success-Response:
 *      {
 *          "messageUri": "/controllerConfig",
 *          "controllerClass": "7"
 *          "modules" : [
 *                        {
 *                           "address": "0",
 *                           "class": "1"
 *                        },
 *                        {
 *                            "address": "1",
 *                            "class": "1"
 *                        },
 *                      ]
 *      }
 */
/**
 * @api {get} /api/notification/controller Controller status notification.
 * @apiName ControllerStatusChange
 * @apiGroup Notifications
 *
 * @apiParam {Number} serialNumber The controller's serial number (chip ID).
 * @apiParam {Number=0,1,2,3,4} status The current state of the controller  1=Unknown, 2=Offline, 3=Online, 4=Restarting, 5=Connected.
 * @apiParam {Number} pingLength The total time (in milliseconds) for the last ping to make its round trip from the server to the controller.
 * @apiDescription Notification message sent when a controller's status changes.
 * @apiSuccessExample {json} Success-Response:
 *      {
 *          "messageUri": "/api/notification/controller",
 *          "serialNumber": "123456"
 *          "status": "3"
 *          "pingLength": "62"
 *      }
 */
/////////////////////////////////////////////////////////////////////

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
