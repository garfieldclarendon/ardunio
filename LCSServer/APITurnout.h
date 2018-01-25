#ifndef APITURNOUT_H
#define APITURNOUT_H

#include <QObject>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"

/**
 * @api {get} /api/lock_device:deviceID,lock Lock Turnout
 * @apiName LockTurnout
 * @apiGroup Turnout
 *
 * @apiParam {Number} deviceID The turnout's Device ID.
 * @apiParam {Number=0,1} lock Flag which unlocks or locks the turnout.  0 = unlock, 1 = lock.
 * @apiDescription Locks/unlocks the turnout.  If the lock parameter is set to 1, the turnout will ignore future TRN_ACTIVATE and TRN_ACTIVATE_ROUTE messages until
 * another lock message is sent with the lock flag set to 0.
 * @apiExample Example usage:
 * http://localhost:8080/api/lock_device?deviceID=7&lock=1
 */

/**
 * @api {get} /api/activate_turnout:deviceID,turnoutState Activate Turnout
 * @apiName ActivateTurnout
 * @apiGroup Turnout
 *
 * @apiParam {Number} deviceID The turnout's Device ID.
 * @apiParam {Number=1,3} [turnoutState]  The desired state to set the turnout to.
 * @apiDescription Sets the turnout to the desired state (direction) 1 = Normal 2 = Diverging.  If no state is specified, the turnout is toggled between normal and thrown.
 * @apiExample Example usage:
 * http://localhost:8080/api/activate_turnout?deviceID=7&turnoutState=3
 */

/**
 * @api {get} /controller/device/config:deviceID Download Configuration
 * @apiName TurnoutConfiguration
 * @apiGroup Turnout
 *
 * @apiParam {Number} deviceID The turnout's Device ID.
 * @apiDescription Download the turnout's configuration.  INPUTPIN and MOTORPIN entries control how the device handles the pin.  If set to 1, the pin order is reversed.
 * This is useful if the wires are connected backwards to the motor and/or the input pins.  Usually, both of these entries are set together, meaning, if MOTORPIN is set to 1 set INPUTPIN to 1 as well.
 * The routes list is used by the device to determine what direction the turnout should be set to in response to a TRN_ACTIVATE_ROUTE message.
 * @apiExample Example usage.  This example returns the configuration for a Turnout Device:
 * http://localhost:8080/controller/device/config?deviceID=1
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "INPUTPIN": "1",
 *          "MOTORPIN": "1",
 *          "deviceClass": "1",
 *          "routes": [{
 *                  "routeID": "1",
 *                  "turnoutState": "1"
 *              }, {
 *                  "routeID": "2",
 *                  "turnoutState": "1"
 *              }, {
 *                  "routeID": "3",
 *                  "turnoutState": "3"
 *              }, {
 *                  "routeID": "4",
 *                  "turnoutState": "3"
 *              }
 *          ]
 *      }
 */

class APITurnout : public QObject
{
    Q_OBJECT
public:
    explicit APITurnout(QObject *parent = nullptr);

signals:

public slots:
    void handleActivateTurnoutUrl(const APIRequest &request, APIResponse *response);
};

#endif // APITURNOUT_H
