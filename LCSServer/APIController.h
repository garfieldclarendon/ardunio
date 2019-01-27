#ifndef APICONTROLLER_H
#define APICONTROLLER_H

#include <QObject>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"

/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation
/**
 * @apiDefine APINotifications Notifications
 * API notifications are sent as a JSON text message through a separate Web Socket connection initiated by the client.  In order for the client to receive notification
 * messages, this connection must remain open/connected during the entire session. Here's an <a href="http://APITest.entrydns.org:8080/web/TestRoutes.html">example</a>.
 */

/**
 * @api {get} /controller/config:serialNumber Download Controller Configuration
 * @apiName ControllerConfiguration
 * @apiGroup Controller
 *
 * @apiParam {Number} serialNumber The controller's Serial number.
 * @apiDescription Download the controller's configuration which includes the controller's ID, list of controllers to notify and a list of connected modules.
 * @apiSuccess {Object} config Controller configuration.
 * @apiSuccess {Number} config.controllerClass Controller's classification.
 * @apiSuccess {Number} config.controllerID Controller's ID.
 * @apiSuccess {Object[]} controllersToNotify List of controllers that need to be notified when device status changes occur.
 * @apiSuccess {Number} controllersToNotify.controllerID Controller's ID.
 * @apiSuccess {Object[]} modules List of connected modules.
 * @apiSuccess {Number} modules.address Module's address.
 * @apiSuccess {Number} modules.class  Module's classification.
 * @apiExample Example usage:
 * http://localhost:8080/controller/config?serialNumber=1546165
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "controllerClass": "7",
 *          "controllerID": "34",
 *          "controllersToNotify": [{
 *                  "controllerID": "35"
 *              }
 *          ],
 *          "modules": [{
 *                  "address": "0",
 *                  "class": "1"
 *              }, {
 *                  "address": "1",
 *                  "class": "1"
 *              }
 *          ]
 *      }
 */

/**
 * @api {get} /controller/module/config:serialNumber,address Download module configuration
 * @apiName ControllerModuleConfiguration
 * @apiGroup Controller
 *
 * @apiParam {Number} serialNumber Controller's serial number.
 * @apiParam {Number=0-7} address module's address.
 * @apiDescription Download the modules configuration which consists of a list of connected devices.
 * @apiSuccess {Object[]} devices List of connected devices.
 * @apiSuccess {Number} devices.c Device's classification.
 * @apiSuccess {Number} devices.id Device's ID.
 * @apiSuccess {Number} devices.p Port/pin to which the device is connected.
 * @apiExample Example usage:
 * http://localhost:8080/controller/module/config?serialNumber=1546165&address=0
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      {
 *          "devices": [{
 *                  "c": "1",
 *                  "id": "3",
 *                  "p": "0"
 *              }, {
 *                  "c": "1",
 *                  "id": "4",
 *                  "p": "1"
 *              }
 *          ],
 *      }
 */

/**
 * @api {get} /controller/device/config:deviceID Download device configuration
 * @apiName DeviceConfiguration
 * @apiGroup Controller
 *
 * @apiParam {Number} deviceID The Device's ID.
 * @apiDescription Download the devices configuration.  The returned data is device type dependent, however, it follows a similar format for each device type.
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

/**
 * @api {get} /controller/notification_list:serialNumber Download controllers-to-notify list
 * @apiName ControllersToNotify
 * @apiGroup Controller
 *
 * @apiParam {Number} serialNumber The controller's serial number.
 * @apiDescription Downloads the controller's controllers-to-notify list
 * @apiSuccess {Object[]} controllersToNotify List of controllers that need to be notified when device status changes occur.
 * @apiSuccess {Number} controllersToNotify.controllerID Controller's ID.
 * @apiExample Example usage:
 * http://localhost:8080/controller/notification_list?serialNumber=1546165
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "controllerID": "35"
 *          }
 *      ]
 */

/**
 * @api {get} /api/controller_module_list:controllerID Get controller's module list
 * @apiName ControllerModuleList
 * @apiGroup Controller
 *
 * @apiParam {Number} [controllerID]  The controller's ID.
 * @apiDescription Returns the list of LCS controller modules for a specific controller.
 * @apiSuccess {Number} address Module's address.
 * @apiSuccess {Number} controllerID Controller's ID the module is assigned to.
 * @apiSuccess {Number} controllerModuleID Module's ID.
 * @apiSuccess {Number=0,1} disable enable/disable the module.
 * @apiSuccess {Number} moduleClass Controller's serial number.
 * @apiSuccess {String} moduleName Module's name.
 * @apiExample Example usage:
 * http://localhost:8080/api/controller_module_list?controllerID=34
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "address": "0",
 *              "controllerID": "34",
 *              "controllerModuleID": "19",
 *              "disable": "0",
 *              "moduleClass": "8",
 *              "moduleName": "CA Aux Panel Input"
 *          }, {
 *              "address": "1",
 *              "controllerID": "34",
 *              "controllerModuleID": "20",
 *              "disable": "0",
 *              "moduleClass": "9",
 *              "moduleName": "CA Aux Panel Output 1"
 *          }, {
 *              "address": "2",
 *              "controllerID": "34",
 *              "controllerModuleID": "23",
 *              "disable": "0",
 *              "moduleClass": "9",
 *              "moduleName": "CA Aux Panel Output 2"
 *          }
 *      ]
*/

/**
 * @api {get} /api/controller_list:controllerID Get controller list
 * @apiName ControllerList
 * @apiGroup Controller
 *
 * @apiParam {Number} [controllerID]  The controller's ID.
 * @apiDescription Returns the list of LCS controllers.
 * @apiSuccess {Number} controllerClass Controller's Class.
 * @apiSuccess {String} controllerDescription Controller's Description.
 * @apiSuccess {Number} controllerID Controller's ID.
 * @apiSuccess {String} controllerName Controller's Name.
 * @apiSuccess {Number} serialNumber Controller's Serial NUmber.
 * @apiSuccess {Number=0,1,2,3} status Controller's current status. 0 = Unknown, 1 = Offline, 2 = Online, 3 = Restarting.
 * @apiSuccess {Number} version Controller's current firmware version.
 * @apiExample Example usage:
 * http://localhost:8080/api/controller_list
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "controllerClass": "7",
 *              "controllerDescription": "",
 *              "controllerID": "34",
 *              "controllerName": "CA Aux Panel",
 *              "serialNumber": "877478",
 *              "status": 0,
 *              "version": "0.0.0"
 *          }, {
 *              "controllerClass": "7",
 *              "controllerDescription": "",
 *              "controllerID": "33",
 *              "controllerName": "CA East End Multi",
 *              "serialNumber": "1593808",
 *              "status": 2,
 *              "version": "0.0.0"
 *          }, {
 *              "controllerClass": "7",
 *              "controllerDescription": "",
 *              "controllerID": "31",
 *              "controllerName": "CA Panel 1",
 *              "serialNumber": "877670",
 *              "status": 2,
 *              "version": "0.0.0"
 *          }
 *      ]
*/

/**
 * @api {get} /api/send_controller_reset:serialNumber Reset Controller
 * @apiName ControllerReset
 * @apiGroup Controller
 *
 * @apiParam {Number} [serialNumber]  The controller's serial number.  If 0 or excluded, all controllers will restart.
 * @apiDescription Sends a SYS_REBOOT_CONTROLLER broadcast UDP message instructing the controller(s) to restart.
 * @apiExample Example usage:
 * http://localhost:8080/api/send_controller_reset?serialNumber=1546165
 */

/**
 * @api {get} /api/send_controller_reset_config:serialNumber Reset Controller's configuration
 * @apiName ControllerConfigurationReset
 * @apiGroup Controller
 *
 * @apiParam {Number} [serialNumber]  The controller's serial number.  If 0 or excluded, all controllers will delete their configuration data.
 * @apiDescription Sends a SYS_RESET_CONFIG broadcast UDP message instructing the controller(s) to delete all configuration data.  Once the data is deleted, the controller will restart.  Upon restarting, the controller re-downloads all configuration data.
 * @apiExample Example usage:
 * http://localhost:8080/api/send_controller_reset_config?serialNumber=1546165
 */

/**
 * @api {get} /api/send_controller_reset_notification_list:serialNumber Reset Controller's controllers-to-notify list
 * @apiName ControllerConfigurationReset
 * @apiGroup Controller
 *
 * @apiParam {Number} [serialNumber]   The controller's serial number.  If 0 or excluded, all controllers will delete their controllers-to-notify list.
 * @apiDescription Sends a SYS_RESET_NOTIFICATION_LIST broadcast UDP message instructing the controller(s) to delete controllers-to-notify list.  The controller responds by sending a /controller/notification_list to the application server.
 * @apiExample Example usage:
 * http://localhost:8080/api/send_controller_reset_notification_list?serialNumber=1546165
 */


/**
 * @api {get} /api/notification/controller Controller Status Change
 * @apiName ControllerStatusChangeNotification
 * @apiGroup APINotifications
 * @apiSampleRequest off
 *
 * @apiDescription Notification message sent when a controller's state changes (online, offline or restarting)
 * @apiSuccess {String} url Notification url.
 * @apiSuccess {Number} serialNumber Controller's serial number.
 * @apiSuccess {Number=1,2,3} status Controller's new status.  1 = Offline, 2 = Online, 3 = Restarting
 * @apiSuccessExample {json} Success-Response:
 *      {
 *              "url": "/api/notification/controller",
 *              "serialNumber": "1546165",
 *              "status": "2"
 *      }
 *
 */

/////////////////////////////////////////////////////////////////////

class QTcpSocket;

/// APIController
/// \brief  API handler class for a LCS controller.
///
/// This class handles all API requests for a controller.  Two types of requests are made as it relates to a LCS controller; "/API/..." requests sent from an
/// external source like a web page or an application and a "/controller/..." requests sent by a LCS controller to download information from the application server.
class APIController : public QObject
{
    Q_OBJECT
public:
    /// Contstructor
    /// Creates a controller API handler.  The constructor registers a \ref UrlHandler "Url Handler" objects.
    /// See the API documentation for information on the URL's handled by this class.
    ///
    explicit APIController(QObject *parent = nullptr);


signals:
    ///Notifies interested parties that a controller's status has changed.
    void sendNotificationMessage(const QString &url, const QJsonObject &obj);

public slots:
    ///Monitors the ControllerManager for controller status changes
    /// @param serialNumber long Serial number of the controller.
    /// @param newStatus ControllerStatusEnum The controller's new status.
    void onControllerStatusChanged(long serialNumber, ControllerStatusEnum newStatus, const QString &version);

    // Controller API Handlers
    /// API "/controller/config"
    /// Downloads a controller's configuration
    /// @param request APIRequest containing the url of the request including the serial number of the controller.
    /// @param response APIResponse with the payload set to the controller's configuration information in JSON format.
    void handleConfigUrl(const APIRequest &request, APIResponse *response);

    /// API "/controller/module/config"
    /// Downloads a controller module's configuration
    /// @param request APIRequest containing the url of the request including the serial number of the controller and the address of the module.
    /// @param response APIResponse with the payload set to the controller module's configuration information in JSON format.
    void handleModuleConfigUrl(const APIRequest &request, APIResponse *response);

    /// API "/controller/device/config"
    /// Downloads a device's configuration
    /// @param request APIRequest containing the url of the request including the deviceID.
    /// @param response APIResponse with the payload set to the device's configuration information in JSON format.
    void handleDeviceConfigUrl(const APIRequest &request, APIResponse *response);

    /// API "/controller/notification_list"
    /// Downloads a controller's controllers-to-notify list.
    /// @param request APIRequest containing the url of the request including the controller's serial number.
    /// @param response APIResponse with the payload set to the controller's list of controllers-to-notify in JSON format.
    void handleGetNotificationListUrl(const APIRequest &request, APIResponse *response);

    /// API "/controller/firmware"
    /// Downloads a controller's firmware.
    /// @param request APIRequest containing the url of the request including the controller's serial number.
    /// @param response APIResponse not used.
    void handleDownloadFirmwareUrl(const APIRequest &request, APIResponse *response);

    /// API "/controller/route_list"
    /// Downloads a controller's firmware.
    /// @param request APIRequest containing the url of the request including the route's id.
    /// @param response APIResponse with the payload set to the route's turnout list in JSON format.
    void handleGetRouteList(const APIRequest &request, APIResponse *response);

    /// API "/api/controller_list"
    /// Downloads a list of all controllers.
    /// @param request APIRequest containing the url of the request including the controller's serial number.
    /// @param response APIResponse with the payload set to the list of controllers in JSON format.
    void handleGetControllerList(const APIRequest &request, APIResponse *response);

    /// API "/api/controller_module_list"
    /// Downloads a list of modules for a controller or a specific module.
    /// @param request APIRequest containing the url of the request including the controller's ID or a specific module ID.
    /// @param response APIResponse with the payload set to the list of controller modules in JSON format.
    void handleGetControllerModuleList(const APIRequest &request, APIResponse *response);

    /// API "/api/send_controller_reset"
    /// Sends a SYS_REBOOT_CONTROLLER broadcast UDP message instructing the controller(s) to restart.
    /// @param request APIRequest containing the url of the request including the controller's serial number.  If the serial number is less than 1, all controllers are restarted.
    /// @param response APIResponse unused.
    void handleControllerReset(const APIRequest &request, APIResponse *response);

    /// API "/api/send_controller_reset_config"
    /// Sends a SYS_RESET_CONFIG broadcast UDP message instructing the controller(s) to delete all configuration data.
    /// @param request APIRequest containing the url of the request including the controller's serial number.  If the serial number is less than 1, all controllers delete their configuration data.
    /// @param response APIResponse unused.
    void handleControllerResetConfig(const APIRequest &request, APIResponse *response);

    /// API "/api/send_controller_reset_notification_list"
    /// Sends a SYS_RESET_NOTIFICATION_LIST broadcast UDP message instructing the controller(s) to delete controllers-to-notify list.  The controller responds by sending a /controller/notification_list request to the application server.
    /// @param request APIRequest containing the url of the request including the controller's serial number.  If the serial number is less than 1, all controllers delete their controllers-to-notify list.
    /// @param response APIResponse unused.
    void handleResetNotificationList(const APIRequest &request, APIResponse *response);

    /// API "/api/send_controller_firmware"
    /// Sends a SYS_DOWNLOAD_FIRMWARE broadcast UDP message instructing the controller(s) to download the firmware.  The controller responds by sending a /controller/firmware request to the application server.
    /// @param request APIRequest containing the url of the request including the controller's serial number.  If the serial number is less than 1, all controllers download the new firmware.
    /// @param response APIResponse unused.
    void handleSendFirmware(const APIRequest &request, APIResponse *response);

private:
    /// Creates the JSON notification message containing the serialNumber and the new status of the controller that changed.
    /// @param serialNumber long Serial number of the controller.
    /// @param status ControllerStatusEnum The controller's new status.
    void createAndSendNotificationMessage(long serialNumber, ControllerStatusEnum status, const QString &version);
    /// Returns the contents of a file as a QByteArray
    /// @param fileName QString containing the file's name.
    QByteArray getFile(const QString &fileName);

};

#endif // APICONTROLLER_H
