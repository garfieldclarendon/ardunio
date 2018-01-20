#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include <QObject>
#include <QMap>
#include <QWebSocketServer>

#include "GlobalDefs.h"
#include "controllermessage.h"
#include "UDPMessage.h"

class ControllerEntry;
class QWebSocket;
class QJsonObject;
class QTimer;

/**
 * @api {get} /api/notification/controller Controller Status Change
 * @apiName ControllerStatusChangeNotification
 * @apiGroup Notifications
 *
 * @apiDescription Notification message sent when a controller's state changes (online, offline or restarting)
 * @apiSuccess {Number} serialNumber Controller's serial number.
 * @apiSuccess {Number=1,2,3} status Controller's new status.  1 = Offline, 2 = Online, 3 = Restarting
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *              "serialNumber": "1546165"
 *              "status": "2"
 *      }
 *
 */

class ControllerManager : public QObject
{
    Q_OBJECT
    explicit ControllerManager(QObject *parent = 0);
public:
    ~ControllerManager(void);

    static ControllerManager *instance(void);
    int getConnectionSerialNumber(int index) const;
    void getConnectedInfo(int serialNumber, QString &version, ControllerStatusEnum &status);
    int getConnectionCount(void) const { return m_controllerMap.keys().count(); }

signals:
    void controllerConnected(int index);
    void controllerDisconnected(int index);
    void controllerAdded(int serialNumber);
    void controllerRemoved(int serialNumber);
    void sendNotificationMessage(const QString &uri, const QJsonObject &obj);

public slots:
    unsigned long getSerialNumber(int controllerID);

protected slots:
    void newUDPMessage(const UDPMessage &message);

private:
    void createAndSendNotificationMessage(long serialNumber, ControllerStatusEnum status);

    static ControllerManager *m_instance;

    QMap<int, ControllerEntry *> m_controllerMap;
};

#endif // CONTROLLERMANAGER_H
