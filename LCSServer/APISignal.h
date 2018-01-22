#ifndef APISIGNAL_H
#define APISIGNAL_H

#include <QObject>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"

/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation
/**
 * @api {get} /api/signal_aspect_list:deviceID Signal Aspect List
 * @apiName SignalAspectList
 * @apiGroup Signal
 *
 * @apiParam {Number} deviceID The signal's Device ID.
 * @apiDescription Returns a list aspects defined for a signal sorted by sortIndex.  A Signal device uses this list to determine what state the LED's of a
 * signal head should be set to (on, off or flashing).  The signal starts at the first entry in this list and evaluates each signal condition entry assigned
 * to this aspect (see /api/signal_condition_list).  If all conditions of an aspect entry evaluate to true, the signal is set to that aspect.  If none of
 * the aspects evaluate to true, the signal sets the red LED on and all others off.
 * @apiSuccess {Number} deviceID Signal's device ID.
 * @apiSuccess {Number=0,1,2} greenMode Green LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number} redMode Red LED state.  0 = off, 1 = on, 2 = flashing.
 * @apiSuccess {Number} signalAspectID signalAspect's ID.
 * @apiSuccess {Number} sortIndex sort order.
 * @apiSuccess {Number} yellowMode Yellow LED state.  0 = off, 1 = on, 2 = flashing.
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
 * comparing the device's current state specified by deviceID to the entry's deviceState using the conditionOperand.  If the comparison is successful
 * the Signal Condition is considered to be true/valid.  All conditions in the list must evaluate to true/valid for the aspect to be selected.
 * @apiSuccess {Number=0,1} conditionOperand Operand used for comparison.  0 = Equals, 1 = Not Equals.
 * @apiSuccess {Number=0,1,2} deviceID The ID of the device to be monitored.
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
