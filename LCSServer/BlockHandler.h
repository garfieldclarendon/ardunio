#ifndef BLOCKHANDLER_H
#define BLOCKHANDLER_H

#include <QMap>
#include <QMutex>

#include "GlobalDefs.h"
#include "DeviceHandler.h"

/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation.
// Device specific messages/notifcations are documented in their respective files
/**
 * @api {put} /controller/module:serialNumber,address,classCode,action Message from the Block Module of state change in the monitored block.
 * @apiName BlockStateChanged
 * @apiGroup Block
 *
 * @apiParam {Number} serialNumber The controller's serial number (chip ID).
 * @apiParam {Number=0-8} address The sending module's address.
 * @apiParam {Number} classCode The sending module's classification.
 * @apiParam {Number} action The action requested.  0=Get,1=Add,2=Update,3=Delete
 * @apiDescription Sent by a controller's BlockModule indicating the state of the block it's monitoring has changed.
 * @apiSuccessExample {json} Success-Response:
 *      {
 *          "action": "2"
 *          "messageUri": "/controller/module",
 *          "class": "8"
 *          "address": "0"
 *          "blocks": [
 *                      {
 *                          "pin": 1
 *                          "pinState": 0
 *                      },
 *                      {
 *                          "pin": 4
 *                          "pinState": 1
 *                      },
 *                    ]
 *      }
 */
/////////////////////////////////////////////////////////////////////

class BlockHandler : public DeviceHandler
{
    Q_OBJECT
public:
    explicit BlockHandler(QObject *parent = 0);

    // DeviceHandler interface
public:

signals:
    void sendNotificationMessage(const QString &uri, const QJsonObject &obj);

public slots:
    void newMessage(int serialNumber, int address, DeviceClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);
    void controllerRemoved(int serialNumber);

private:
    void getdeviceID(int serialNumber, int address, int port, int &deviceID);
    void setCurrentState(int deviceID, BlockState newState);
    void createAndSendNotificationMessage(int deviceID, BlockState newState);
    void updateBlockState(const QJsonObject &json, int serialNumber, int address);

    QMutex m_mapMutex;
    QMap<int, BlockState> m_blockStates;
};

#endif // BLOCKHANDLER_H
