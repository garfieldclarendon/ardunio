#ifndef APIDEVICE_H
#define APIDEVICE_H

#include <QObject>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"
/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation
/**
 * @api {get} /api/device_list:serialNumber,moduleID,controllerID,classCode Get a list of devices
 * @apiName GetDeviceList
 * @apiGroup Device
 *
 * @apiParam {Number} [serialNumber] (optional) filter device list by a specific controller's serial number.
 * @apiParam {Number}[moduleID] (optional) filter device list by a specific controller module's ID.
 * @apiParam {Number} [controllerID] (optional) filter device list by a specific controller's ID.
 * @apiParam {Number} [classCode] (optional) filter device list by a specific device classification.
 * @apiDescription Returns a list of devices.  If no parameters are supplied, all devices are returned.
 * @apiSuccess {Number} address  Address of the module the device is connected to.
 * @apiSuccess {Number} controllerID Controller ID of the controller the device is connected to.
 * @apiSuccess {Number} controllerModuleID Controller Module ID of the controller module the device is connected to.
 * @apiSuccess {Number} deviceClass Device classification.
 * @apiSuccess {String} deviceDescription Device's description.
 * @apiSuccess {Number} deviceID Device's ID.
 * @apiSuccess {String} deviceName Device's name
 * @apiSuccess {Number} deviceState  Device's current state.
 * @apiSuccess {Number} moduleClass  Classification of the controller module to which the device is connected to.
 * @apiSuccess {Number} port  Port/pin the device is connected to.
 * @apiSuccess {Number} serialNumber  Serial Number of the controller the device is connected to.
 * @apiExample Example usage:
 * http://localhost:8080/api/device_list?controllerID=30
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "address": "0",
 *              "controllerID": "30",
 *              "controllerModuleID": "8",
 *              "deviceClass": "5",
 *              "deviceDescription": "",
 *              "deviceID": "19",
 *              "deviceName": "GK Mine Signal",
 *              "deviceState": 0,
 *              "moduleClass": "5",
 *              "port": "0",
 *              "serialNumber": "585680"
 *          }
 *      ]
 */

/**
 * @api {get} /api/device_property_list:deviceID Get deviceProperty entries for a given device
 * @apiName GetDevicePropertyList
 * @apiGroup Device
 *
 * @apiParam {Number} deviceID device's ID.
 * @apiDescription Returns a list of deviceProperty entries for a given device..
 * @apiSuccess {Number} deviceID  Device's ID.
 * @apiSuccess {Number} id deviceProperty's id.
 * @apiSuccess {Number} key Device Property key.
 * @apiSuccess {Number} value Device Property value.
 * @apiExample Example usage:
 * http://localhost:8080/api/device_property_list?deviceID=1
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "deviceID": "1",
 *              "id": "243",
 *              "key": "MOTORPIN",
 *              "value": "1"
 *          }, {
 *              "deviceID": "1",
 *              "id": "244",
 *              "key": "INPUTPIN",
 *              "value": "1"
 *          }
 *      ]
 */

/**
 * @api {get} /api/send_device_config:deviceID Reset device's configuration
 * @apiName DeviceConfigReset
 * @apiGroup Device
 *
 * @apiParam {Number} [deviceID] (optional) The device's id.  If 0 or excluded, all devices will reset their configuration data.
 * @apiDescription Sends a SYS_RESET_DEVICE_CONFIG broadcast UDP message instructing the device(s) to re-download its configuration data.
 * @apiExample Example usage:
 * http://localhost:8080/api/send_device_config?deviceID=1
 */

/**
 * @api {get} /api/copy_device:deviceID Copy a device's data to a new device entry
 * @apiName CopyDevice
 * @apiGroup Device
 *
 * @apiParam {Number} deviceID The device's id to be coppied.
 * @apiDescription Creates a copy of an existing device.  The new entry is an exact copy of the original except for the id field which is set to the new entry's id value.
 * @apiSuccess {Number} controllerModuleID  Controller Module's ID to which the device is connected.
 * @apiSuccess {Number} deviceClass Device's classification.
 * @apiSuccess {String} deviceDescription Device's description.
 * @apiSuccess {String} deviceName Device Device's name.
 * @apiSuccess {Number} id Device's new ID.
 * @apiSuccess {Number} port/pin Port to which the device is connected.
 * @apiExample Example usage:
 * http://localhost:8080/api/copy_device?deviceID=1
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "controllerModuleID": "4",
 *              "deviceClass": "1",
 *              "deviceDescription": "",
 *              "deviceName": "TY30-1",
 *              "id": "82",
 *              "port": "0"
 *          }
 *      ]
 */

/**
 * @api {get} /api/notification/device Device Status Change
 * @apiName DeviceStatusChangeNotification
 * @apiGroup APINotifications
 *
 * @apiDescription Notification message sent when a device's state changes.
 * @apiSuccess {String} url Notification url.
 * @apiSuccess {Number} deviceID Device's ID.
 * @apiSuccess {Number} deviceState Device's new state
 * @apiSuccess {Number=0,1} locked Device's locked state.  0 = unlocked, 1 = locked.
 * @apiSuccessExample {json} Success-Response:
 *      {
 *              "url": "/api/notification/device"
 *              "deviceID": "1"
 *              "deviceState": "2"
 *              "locked": "0"
 *      }
 *
 */
class APIDevice : public QObject
{
    Q_OBJECT
public:
    explicit APIDevice(QObject *parent = nullptr);

signals:
    void sendNotificationMessage(const QString &url, const QJsonObject &obj);

public slots:
    void handleGetDeviceList(const APIRequest &request, APIResponse *response);
    void handleGetDevicePropertyList(const APIRequest &request, APIResponse *response);
    void handleSendDeviceConfig(const APIRequest &request, APIResponse *response);
    void handleCopyDevice(const APIRequest &request, APIResponse *response);
    void handleLockDevice(const APIRequest &request, APIResponse *response);
    void onDeviceStatusChanged(int deviceID, int status, bool locked);

private:
    void lockTurnout(int deviceID, bool lock);
    void lockSignal(int deviceID, bool lock, PinStateEnum redMode, PinStateEnum greenMode, PinStateEnum yellowMode);
    QJsonArray getDeviceList(long serialNumber, int controllerID, int moduleID, int classCode, int deviceID);
    void copyDeviceProperties(int fromID, int toID);
    void createAndSendNotificationMessage(int deviceID, int newState, bool locked);
};

#endif // APIDEVICE_H
