#ifndef APIDEVICE_H
#define APIDEVICE_H

#include <QObject>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"
/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation
/**
 * @api {get} /api/device_list:serialNumber,moduleID,controllerID,classCode,deviceID Get a list of devices
 * @apiName GetDeviceList
 * @apiGroup Device
 *
 * @apiParam {Number} [serialNumber]  filter device list by a specific controller's serial number.
 * @apiParam {Number}[moduleID]  filter device list by a specific controller module's ID.
 * @apiParam {Number} [controllerID]  filter device list by a specific controller's ID.
 * @apiParam {Number} [classCode]  filter device list by a specific device classification.
 * @apiParam {Number} [deviceID]  filter device list by a specific device ID.
 * @apiDescription Returns a list of devices.  If no parameters are supplied, all devices are returned.
 * @apiSuccess {Number} deviceClass Device classification.
 * @apiSuccess {String} deviceDescription Device's description.
 * @apiSuccess {Number} deviceID Device's ID.
 * @apiSuccess {String} deviceName Device's name
 * @apiSuccess {Number} deviceState  Device's current state.
 * @apiExample Example usage:
 * http://localhost:8080/api/device_list?controllerID=30
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "deviceClass": "5",
 *              "deviceDescription": "",
 *              "deviceID": "19",
 *              "deviceName": "GK Mine Signal",
 *              "deviceState": 0,
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
 * @api {get} /api/module_device_port_list:deviceID,moduleID Get module/port entries for a given device
 * @apiName GetModuleDevicePortList
 * @apiGroup Device
 *
 * @apiParam {Number} [deviceID} device's ID.
 * @apiParam {Number} [modleID] controllerModule's ID.
 * @apiDescription Returns a list of moduleDevicePort entries for a given device or module.  This table cross-references
 * a device to one or more modules.  This allows a single device entry to be used with multiple modules; useful for PanelInputDevice and PanelOutputDevice, etc.
 * @apiSuccess {Number} id moduleDevicePort's ID.
 * @apiSuccess {Number} deviceID  device's ID.
 * @apiSuccess {Number} controllerModuleID controllerModule's id.
 * @apiSuccess {String} moduleName controllerModule's name.
 * @apiSuccess {Number} moduleClass controllerModule's class.
 * @apiSuccess {String} deviceName device's name.
 * @apiSuccess {Number} deviceClass device's class.
 * @apiSuccess {Number} port port the device is connected to.
 * @apiExample Example usage:
 * http://localhost:8080/api/device_property_list?deviceID=11
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "controllerModuleID": "13",
 *              "deviceClass": "6",
 *              "deviceID": "11",
 *              "deviceName": "Block 30-1",
 *              "id": "1",
 *              "moduleClass": "8",
 *              "moduleName": "Blocks 30-1, 30-9, 31-1, 31-9",
 *              "port": "7"
 *          }
 *      ]
 */

/**
 * @api {get} /api/send_device_config:deviceID Reset device's configuration
 * @apiName DeviceConfigReset
 * @apiGroup Device
 *
 * @apiParam {Number} [deviceID]  The device's id.  If 0 or excluded, all devices will reset their configuration data.
 * @apiDescription Sends a SYS_RESET_DEVICE_CONFIG broadcast UDP message instructing the device(s) to re-download its configuration data.
 * @apiExample Example usage:
 * http://localhost:8080/api/send_device_config?deviceID=1
 */

/**
 * @api {post} /api/create_device:createExtra Create a device entry
 * @apiName CreateDevice
 * @apiGroup Device
 *
 * @apiParam {Number} [createExtra] Flag that, when set to true, instructs the system to create additional companion devices.
 * @apiDescription Creates a new device entry.  In addition to the device table entry, any required deviceProperty entries are also created.  If
 * the createExtra flag is set to 1, additional devices are created.  For turnouts,
 * two PanelOutput devices are created for the red and green panel LEDs.  For a block device, one PanelOutput device is created for the yellow LED.
 * @apiSuccess {Number} controllerModuleID  Controller Module's ID to which the device is connected.
 * @apiSuccess {Number} deviceClass Device's classification populated with the supplied deviceClass.
 * @apiSuccess {String} deviceDescription Device's description.
 * @apiSuccess {String} deviceName Device Device's name.
 * @apiSuccess {Number} id Device's new ID.
 * @apiSuccess {Number} port/pin Port to which the device is connected.
 * @apiExample This example creates a new Turnout device entry.  In addition to the device table entry, two entries are also added to the deviceProperty table; MOTORPIN and INPUTPIN with values set to 0:
 * http://localhost:8080/api/create_device?createExtra=1
 *     body:
 *     {
 *       "deviceName": "TY-NEW",
 *       "deviceDescription": "Description",
 *       "deviceClass": "1"
 *     }
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "controllerModuleID": "0",
 *              "deviceClass": "1",
 *              "deviceDescription": "",
 *              "deviceName": "",
 *              "id": "83",
 *              "port": "0"
 *          }
 *      ]
 */

/**
 * @api {get} /api/notification/device Device Status Change
 * @apiName DeviceStatusChangeNotification
 * @apiGroup APINotifications
 * @apiSampleRequest off
 *
 * @apiDescription Notification message sent when a device's state changes.
 * @apiSuccess {String} url Notification url.
 * @apiSuccess {Number} deviceID Device's ID.
 * @apiSuccess {Number} deviceState Device's new state
 * @apiSuccess {Number=0,1} locked Device's locked state.  0 = unlocked, 1 = locked.
 * @apiSuccessExample {json} Success-Response:
 *      {
 *              "url": "/api/notification/device",
 *              "deviceID": "1",
 *              "deviceState": "2",
 *              "locked": "0"
 *      }
 *
 */

/// APIDevice
/// \brief  API handler class for a LCS device.
///
/// This class handles basic API requests for a device.  APISignal, APITurnout and APIRoute handle device-specific requests.
class APIDevice : public QObject
{
    Q_OBJECT
public:
    /// Contstructor
    /// Creates a device API handler.  The constructor registers a \ref UrlHandler "Url Handler" objects.
    /// See the API documentation for information on the URL's handled by this class.
    ///
    explicit APIDevice(QObject *parent = nullptr);

signals:
    ///Notifies interested parties that a device's status or lock state has changed.
    void sendNotificationMessage(const QString &url, const QJsonObject &obj);

public slots:
    /// API "/api/device_list"
    /// Downloads a list of all devices.
    /// @param request APIRequest containing the url of the request including any filtering parameters.
    /// @param response APIResponse with the payload set to the list of devices in JSON format.
    void handleGetDeviceList(const APIRequest &request, APIResponse *response);

    /// API "/api/device_property_list"
    /// Downloads a list of deviceProperty entries for a given deviceID.
    /// @param request APIRequest containing the url of the request including the device's ID.
    /// @param response APIResponse with the payload set to the list of deviceProperties in JSON format.
    void handleGetDevicePropertyList(const APIRequest &request, APIResponse *response);

    /// API "/api/module_device_port_list"
    /// Downloads a list of module/port entries for a given deviceID.
    /// @param request APIRequest containing the url of the request including the device's ID.
    /// @param response APIResponse with the payload set to the list of deviceProperties in JSON format.
    void handleGetModuleDevicePortList(const APIRequest &request, APIResponse *response);

    /// API "/api/send_device_config"
    /// Sends a SYS_RESET_DEVICE_CONFIG broadcast UDP message instructing the device to reset its configuration.
    /// @param request APIRequest containing the url of the request including the device's ID.
    /// @param response APIResponse unused.
    void handleSendDeviceConfig(const APIRequest &request, APIResponse *response);

    /// API "/api/create_device"
    /// Creates a new device entry.  deviceProperty entries (if applicable) are also created for the supplied deviceClass.
    /// @param request APIRequest containing the url of the request including the device's class.
    /// @param response APIResponse with the payload set to the new device data in JSON format.
    void handleCreateDevice(const APIRequest &request, APIResponse *response);


    /// API "/api/lock_device"
    /// Sends a SYS_LOCK_DEVICE broadcast UDP message instructing the device to lock/unlock.
    /// @param request APIRequest containing the url of the request including the device's ID and the lock flag.
    /// @param response APIResponse unused.
    void handleLockDevice(const APIRequest &request, APIResponse *response);

    ///Monitors the DeviceManager for device status changes
    /// @param deviceID int Device ID of the device.
    /// @param status int The device's new status.
    /// @param locked int The device's new locked state.
    void onDeviceStatusChanged(int deviceID, int status, bool locked);

private:
    void lockTurnout(int deviceID, bool lock);
    void lockSignal(int deviceID, bool lock, PinStateEnum redMode, PinStateEnum greenMode, PinStateEnum yellowMode);
    QJsonArray getDeviceList(long serialNumber, int controllerID, int moduleID, int classCode, int deviceID);
    void createAndSendNotificationMessage(int deviceID, int newState, bool locked);
    void createTurnoutExtra(const QString &deviceName, int turnoutID);
    void createBlockExtra(const QString &deviceName, int blockID);
};

#endif // APIDEVICE_H
