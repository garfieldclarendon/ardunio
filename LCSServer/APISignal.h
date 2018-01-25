#ifndef APISIGNAL_H
#define APISIGNAL_H

#include <QObject>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"

/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation

/**
 * @api {get} /api/lock_device:deviceID,lock,redMode,greenMode,yellowMode Lock Signal
 * @apiName LockSignal
 * @apiGroup Signal
 *
 * @apiParam {Number} deviceID The signal's Device ID.
 * @apiParam {Number=0,1} lock Flag which unlocks or locks the signal to its current aspect.
 * @apiSuccess {Number=0,1,2} [redMode]  Red LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number=0,1,2} [greenMode]  Green LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number=0,1,2} [yellowMode]  Yellow LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiDescription Locks/unlocks the signal.  If the lock parameter is set to 1, and redMode, greenMode and/or yellowMode are supplied, the
 * signal's aspect is first updated to values provided and then locked.  If all three modes are not supplied, the missing modes will be set to 0 = off.  If no
 * LED modes are supplied, the signal's current pin settings are "locked" as is.
 * @apiExample In this example, Signal 21 is locked and its aspect set to flashing yellow.
 * http://localhost:8080/api/lock_device?deviceID=21&lock=1&yellowMode=2
 */

/**
 * @api {get} /api/signal_aspect_list:deviceID Signal Aspect List
 * @apiName SignalAspectList
 * @apiGroup Signal
 *
 * @apiParam {Number} deviceID The signal's Device ID.
 * @apiDescription Returns a list aspects defined for a signal sorted by sortIndex.  A Signal device uses this list to determine what state the LED's of a
 * signal head should be set to (on, off or flashing).  The signal starts at the first entry in this list and evaluates each signal condition entry assigned
 * to the aspect (see /api/signal_condition_list).  If all conditions of an aspect entry evaluate to true, processing stops and the signal is set to the aspect.  If none of
 * the aspects evaluate to true, the signal sets the red LED on and all others off.
 * @apiSuccess {Number} deviceID Signal's device ID.
 * @apiSuccess {Number=0,1,2} greenMode Green LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number=0,1,2} redMode Red LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number} signalAspectID signalAspect's ID.
 * @apiSuccess {Number} sortIndex sort order.
 * @apiSuccess {Number=0,1,2} yellowMode Yellow LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiExample Example usage:
 * http://localhost:8080/api/signal_aspect_list?deviceID=21
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "deviceID": "21",
 *              "greenMode": "1",
 *              "redMode": "0",
 *              "signalAspectID": "5",
 *              "sortIndex": "0",
 *              "yellowMode": "0"
 *          }, {
 *              "deviceID": "21",
 *              "greenMode": "2",
 *              "redMode": "1",
 *              "signalAspectID": "6",
 *              "sortIndex": "1",
 *              "yellowMode": "0"
 *          }
 *      ]
 */

/**
 * @api {get} /api/signal_condition_list:aspectID Aspect Condition List
 * @apiName SignalAspectConditionList
 * @apiGroup Signal
 *
 * @apiParam {Number} aspectID The aspect's ID.
 * @apiDescription Returns a list conditions that must be met for the aspect to be valid (selected).  All conditions in this list are evaluated by
 * comparing the device's current state specified by deviceID to the entry's deviceState using the conditionOperand (equals or not equals).  If the comparison is successful
 * the Signal Condition is considered to be true/valid.  All conditions in this list must evaluate to true for the aspect to be selected.
 * @apiSuccess {Number=0,1} conditionOperand Operand used for comparison.  0 = Equals, 1 = Not Equals.
 * @apiSuccess {Number} deviceID The ID of the device to be monitored.
 * @apiSuccess {Number} deviceState The state the monitored device is compared to.  When evaluated, this value is compared to the device's current state.
 * @apiSuccess {Number} signalAspectID signalAspect's ID.
 * @apiSuccess {Number} signalConditionID signalCondition's ID.
 * @apiExample Example usage:
 * http://localhost:8080/api/signal_condition_list?aspectID=6
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "conditionOperand": "0",
 *              "deviceID": "17",
 *              "deviceState": "3",
 *              "signalAspectID": "6",
 *              "signalConditionID": "19"
 *          }, {
 *              "conditionOperand": "0",
 *              "deviceID": "18",
 *              "deviceState": "3",
 *              "signalAspectID": "6",
 *              "signalConditionID": "20"
 *          }, {
 *              "conditionOperand": "0",
 *              "deviceID": "11",
 *              "deviceState": "1",
 *              "signalAspectID": "6",
 *              "signalConditionID": "21"
 *          }, {
 *              "conditionOperand": "0",
 *              "deviceID": "5",
 *              "deviceState": "3",
 *              "signalAspectID": "6",
 *              "signalConditionID": "22"
 *          }, {
 *              "conditionOperand": "0",
 *              "deviceID": "6",
 *              "deviceState": "3",
 *              "signalAspectID": "6",
 *              "signalConditionID": "23"
 *          }, {
 *              "conditionOperand": "0",
 *              "deviceID": "8",
 *              "deviceState": "1",
 *              "signalAspectID": "6",
 *              "signalConditionID": "24"
 *          }
 *      ]
*/

/**
 * @api {get} /controller/device/config:deviceID Download Configuration
 * @apiName SignalConfiguration
 * @apiGroup Signal
 *
 * @apiParam {Number} deviceID The signal's Device ID.
 * @apiDescription Download the signal's configuration.  See <a href="#api-Signal-SignalAspectList">/api/signal_aspect_list</a> and <a href="#api-Signal-SignalAspectConditionList">/api/signal_condition_list</a> for more information on how the signal device uses this information.
 * @apiSuccess {Object[]} aspects List of aspects.
 * @apiSuccess {Number} aspects.aspectID Aspect's ID.
 * @apiSuccess {Number=0,1,2} aspects.greenMode Green LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number=0,1,2} aspects.redMode Red LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number=0,1,2} aspects.yellowMode Yellow LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Object[]} devices List of devices to monitor.  The signal depends on the current state of these devices in order to determine
 * the proper aspect to display.  The signal listens for UDP status update messages from these devices and saves their current state.
 * @apiSuccess {Number} devices.deviceID ID of the device to monitor.
 * @apiExample Example usage.  This example returns the configuration for Signal Device 21:
 * http://localhost:8080/controller/device/config?deviceID=21
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "aspects": [{
 *                  "aspectID": "5",
 *                  "greenMode": "1",
 *                  "redMode": "0",
 *                  "yellowMode": "0"
 *              }, {
 *                  "aspectID": "6",
 *                  "greenMode": "2",
 *                  "redMode": "1",
 *                  "yellowMode": "0"
 *              }
 *          ],
 *          "devices": [{
 *                  "deviceID": "17"
 *              }, {
 *                  "deviceID": "18"
 *              }, {
 *                  "deviceID": "12"
 *              }, {
 *                  "deviceID": "5"
 *              }, {
 *                  "deviceID": "6"
 *              }, {
 *                  "deviceID": "8"
 *              }, {
 *                  "deviceID": "11"
 *              }
 *          ]
 *      }
 */

/**
 * @api {get} /controller/device/config:aspectID Download Aspect Configuration
 * @apiName SignalAspectConfiguration
 * @apiGroup Signal
 *
 * @apiParam {Number} aspectID The aspect's ID.
 * @apiDescription Download the configuration information for a signal aspect.  See <a href="#api-Signal-SignalAspectList">/api/signal_aspect_list</a> and <a href="#api-Signal-SignalAspectConditionList">/api/signal_condition_list</a> for more information.
 * @apiSuccess {Number} aspectID Aspect's ID.
 * @apiSuccess {Object[]} conditions List of conditions.  See <a href="#api-Signal-SignalAspectList">/api/signal_aspect_list</a> for more information.
 * @apiSuccess {Number=0,1} conditions.conditionOperand Operand used for comparison.  0 = Equals, 1 = Not Equals.
 * @apiSuccess {Number} conditions.deviceID Device ID.
 * @apiSuccess {Number} conditions.deviceState Device state.
 * @apiSuccess {Number=0,1,2} greenMode Green LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number=0,1,2} redMode Red LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number=0,1,2} yellowMode Yellow LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiExample Example usage.  This example returns the configuration for Signal Aspect 6:
 * http://localhost:8080/controller/device/config?aspectID=6
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "aspectID": "6",
 *          "conditions": [{
 *                  "conditionOperand": "0",
 *                  "deviceID": "17",
 *                  "deviceState": "3"
 *              }, {
 *                  "conditionOperand": "0",
 *                  "deviceID": "18",
 *                  "deviceState": "3"
 *              }, {
 *                  "conditionOperand": "0",
 *                  "deviceID": "11",
 *                  "deviceState": "1"
 *              }, {
 *                  "conditionOperand": "0",
 *                  "deviceID": "5",
 *                  "deviceState": "3"
 *              }, {
 *                  "conditionOperand": "0",
 *                  "deviceID": "6",
 *                  "deviceState": "3"
 *              }, {
 *                  "conditionOperand": "0",
 *                  "deviceID": "8",
 *                  "deviceState": "1"
 *              }
 *          ],
 *          "greenMode": "2",
 *          "redMode": "1",
 *          "yellowMode": "0"
 *      }
*/

class APISignal : public QObject
{
    Q_OBJECT
public:
    explicit APISignal(QObject *parent = nullptr);

signals:

public slots:
    void handleGetSignalAspectList(const APIRequest &request, APIResponse *response);
    void handleGetSignalConditionList(const APIRequest &request, APIResponse *response);
};

#endif // APISIGNAL_H
