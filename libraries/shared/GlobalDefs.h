#pragma once
#include "Local.h"
/** @file */

const unsigned char MajorVersion = 3;
const unsigned char MinorVersion = 0;
const unsigned char BuildVersion = 32;

/** When PROJECT_DEBUG is defined, DEBUG_PRINT will print output to the serial port.  Otherwise, DEBUG_PRINT does nothing (for release builds)  */
#ifdef PROJECT_DEBUG
#define DEBUG_PRINT(...) Serial.printf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) // nothing to do here
#endif

/** Maximum I2C BUS RETRY COUNT */
#define MAX_I2C_RETRY 3
/** Total turnouts per module */
#define MAX_TURNOUTS 2
/** Total modules per controller */
#define MAX_MODULES 8
/** Total devices per module */
#define MAX_DEVICES 16
/** Maximum routes a turnout can belong to */
#define MAX_ROUTE_ENTRIES 5
/** Maximum devices (turnout and blocks) monitored by a Signal device */
#define MAX_SIGNAL_DEVICES 20
/** Maximum conditions per Signal aspect */
#define MAX_SIGNAL_CONDITIONS 10

/** Input de-bounce timeout interval */
#define TIMEOUT_INTERVAL 200
/** One minute heatbeat broadcast message interval */
#define HEARTBEAT_INTERVAL 60000

    /// This enum describes the current status of a controller
enum ControllerStatusEnum
{
    ControllerStatusUnknown, ///< The controller's current status cannot be determined.
    ControllerStatusOffline, ///< The controller is offline.
    ControllerStatusOnline, ///< The controller is online (replaces ControllerStatusConnected).
    ControllerStatusRestarting, ///< The controller is restarting.
    ControllerStatusConected, ///< No longer used.  Kept for backward compatability.
	ControllerStatusFirmwareUpdate ///< Controller is downloading and updating its firmware.
};

	/// This enum describes the connection between two condition compares
enum ConditionConnectionEnum
{
	ConnectionAND,
	ConnectionOR
};
    /// This enum describes boolean conditional tests used by the Signal device
enum ConditionEnum
{
    ConditionEquals, ///< Compare two values to see if they are equal.
    ConditionNotEquals ///< Compare two values to see if they are not equal.
};

    /// This enum describes input/output configuration of a microchip pin
enum PinModeEnum
{
    PinUnknown, ///< The pin mode cannot be determined.
    PinInput, ///< The pin is set to input mode.
    PinInputPullup, ///< The pin is set to input mode using the internal pull-up resistor.
    PinOutput, ///< The pin is set to output mode.
    PinOutputOpenDrain ///< The pin is set to output mode.
};

    /// This enum describes current state of a microchip pin
enum PinStateEnum
{
    PinOff, ///< The pin is off (LOW).
    PinOn, ///< The pin is on (HIGH).
    PinFlashing ///< The pin is set to alternate between on and off (LOW and HIGH) states at a timed interval.
};

enum NetActionType
{
    NetActionGet,
    NetActionAdd,
    NetActionUpdate,
    NetActionDelete
};

    /// This enum describes current state of a Turnout Device
enum TurnoutState
{
  TrnUnknown, ///< The turnout state cannot be determined.
  TrnNormal, ///< The turnout is set to it's normal route (usually the straight direction).
  TrnToDiverging, ///< The turnout is in motion, changing from the normal route to the diverging route.
  TrnDiverging, ///< The turnout is set to the diverging route.
  TrnToNormal ///< The turnout is in motion, changing from the diverging route to the normal route.
};

    /// This enum describes current state of a Block Detection Device
enum BlockState
{
    BlockUnknown, ///< The block state cannot be determined.
    BlockClear, ///< The block is clear.
    BlockOccupied ///< The block is occupied.
};

    /// This enum describes the classification of a Controller
enum ControllerClassEnum
{
    ControllerUnknown, ///< The controller's current classification is unknown.
    ControllerMulti = 7, ///< Multi-Module Controller.  Supports up to 8 devices connected by an I2C serial interface.
    ControllerTurnout = 1, ///< Turnout Controller which is a Controller + Turnout Module combined on one board.
    ControllerSemaphore = 5, ///< Semaphore Signal Controller which is a Controller + Turnout module combined on one board.
    ControllerServer = 10, ///< Application Server either a PC desktop or a Raspberry Pi.
    ControllerApp ///< Support/Configuration application.
};

    /// This enum describes the classification of a Device
enum DeviceClassEnum
{
    DeviceUnknown, ///< The device's current classification is unknown.
    DeviceTurnout, ///< Turnout Device.
    DevicePanelInput, ///< Panel Button Input device.
    DevicePanelOutput, ///< Panel LED Output Module.
    DeviceSignal = 4, ///< Signal Device.
    DeviceSemaphore = 5, ///< Semaphore Signal Device.
    DeviceBlock = 6, ///< Block Device.
    DevicePanel = 7 ///< For older versions--no longer used.
};

    /// This enum describes the classification of a Controller Module
enum ModuleClassEnum
{
    ModuleUnknown, ///< The controller module's current classification is unknown.
    ModuleTurnout = 1, ///< Turnout Module.
    ModuleSemaphore = 5, ///< Semaphore Module which is really a turnout module.
    ModuleInput = 8, ///< Input Module used by Panel Input and Block devices.
    ModuleOutput = 9 ///< Output Module used by Panel Output and Signal devices.
};

/** UDP broadcast address */
const unsigned char UdpBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF};
/** UDP port */
const unsigned int UdpPort = 45457;
/** Local (controller) http server port.  Reserved for future use. */
const unsigned int LocalServerPort = 45455;

/** Configuration addresses for EEPROM */
#define CONTROLLER_ID_ADDRESS 7

// Status Message IDs
/**
 * \defgroup UDPMessageID UDP Message IDs
 * UDP Messages are sent between controllers to execute commands and provide status updates.  Each message is represented by a \ref UDPMessage "UDPMessage"
 * @{
 */
/**
* \ingroup UDPMessageID
* Turnout Device status message
*/
#define TRN_STATUS 1
/**
* \ingroup UDPMessageID
* Block Device status message
*/
#define BLK_STATUS 2
/**
* \ingroup UDPMessageID
* Generic Device status message.  The message payload contains an array of up to 8 device's and their current status.
*/
#define DEVICE_STATUS 3

/**
* \ingroup UDPMessageID
* Place holder; used in for loops to determine the upper bound of status message IDs
*/
#define END_STATUS_MESSAGES 5
/**
* \ingroup UDPMessageID
* Activate Turnout message.  ID property contains the deviceID of the turnout to activate.  Field0 property contains the TurnoutState (TrnUnknown, TrnNormal or TrnDiverging).
* If Field0 is set to TrnUnknown, the command "toggles" the turnout between Normal and Diverging.
*/
#define TRN_ACTIVATE 6
/**
* \ingroup UDPMessageID
* Activate Turnout route message.  ID property contains the routeID.  The Turnout device locates the route in its configuration data and sets the turnout state as defined by the route.
*
*/
#define TRN_ACTIVATE_ROUTE 7

//System Messages
/**
* \ingroup UDPMessageID
* Message Acknowledgement.  ID property and transaction number contains the orignal ID and transaction of the sent message.
*
*/
#define SYS_ACK 10
/**
* \ingroup UDPMessageID
* Controller Online message.  ID property contains the sending controller's controllerID.  The controller checks its list of controllers-to-notify.
* If the controllerID is in the notification list, the controller responds by sending its own SYS_CONTROLLER_ONLINE message back to the sending controller.
* The controller then has all its devices send their current status messages to the notification list.
*
*/
#define SYS_CONTROLLER_ONLINE 11
/**
* \ingroup UDPMessageID
* Application Server Heartbeat message.  The application server sends this message every 60 seconds.  The message contains the server's IP address
* in field[0], field[1], field[2] and field[3] of the message's payload.  Field[4] contains a 1 the first time the controller sends a heartbeat message and 0 on subsequent messages.
* Controllers use the field[4] as a flag to determine when the server is coming online (possibly after a server restart).  The controller will respond by sending
* device status messages to the application server when this flag is set to 1.
*
*/
#define SYS_SERVER_HEARTBEAT 12
/**
* \ingroup UDPMessageID
* Controller Reset Configuration message.  ID property contains the serialNumber of the target controller or 0.  When the ID property is set to 0, all
* controllers will reset their configuration.  In response, controllers will delete all configuration files contained in their flash memory and then
* reboot.  Upon restarting, the controller will detect the absence of the controllers configruation file and start the process of downloading
* its configuration data from the server using a http RESTful API call to the server using the url /controller/config?serialNumber=xxx where xxx
* is the controller's serial number.  See the Controller RESTful API for more information.
*
*/
#define SYS_RESET_CONFIG 13
/**
* \ingroup UDPMessageID
* Reboot Controller message.  ID property contains the serialNumber of the target controller or 0.   When the ID property is set to 0, all
* controllers respond to the message by doing a soft reboot.  This means the microcontroller will reboot.  NOTE: This does not reset the hardware
* connected to the controller, only the microcontroller will reboot.  If the controller is experiencing other issues (modules not responding, etc)
* the soft reboot may not solve the problem and a hard reboot (by turning the controller off and then back on) may be required.
*
*/
#define SYS_REBOOT_CONTROLLER 14
/**
* \ingroup UDPMessageID
* Download Controller firmware message.  ID property contains the serialNumber of the target controller or 0.   When the ID property is set to 0, all
* controllers respond to the message.  The controller will respond by making a http RESTful API call to the server using the url
* /controller/firmware?serialNumber=xxx where xxx is the controller's serial number.  The server will send back the latest version of the controller
* firmware.  Once downloaded, the controller will restart and install the new firmware.  See the Controller RESTful API for more information.
*
*/
#define SYS_DOWNLOAD_FIRMWARE 15
/**
* \ingroup UDPMessageID
* Controller Restarting message.  ID property contains the serialNumber of the controller.   Controllers send this message just before the controller
* is about to restart.
*
*/
#define SYS_RESTARTING 16
/**
* \ingroup UDPMessageID
* Controller Keep-alive message.  No longer used.  Keeping for future use if needed
*
*/
#define SYS_FIRMWARE_UPDATING 17
/**
* \ingroup UDPMessageID
* Controller Firmware upgrade message. The controller is downloading and updating its firmware.
*
*/
#define SYS_KEEP_ALIVE 18
/**
* \ingroup UDPMessageID
* Controller Reset Device Configuration message.  ID property contains the deviceID of the target device.  In response, the target device will
* delete all configuration files contained in the flash memory and then start the process of downloading
* its configuration data from the server using a http RESTful API call to the server using the url /controller/device/config?deviceID=xxx where xxx
* is the device's id number.  See the Controller RESTful API for more information.
*
*/
#define SYS_RESET_DEVICE_CONFIG 19
/**
* \ingroup UDPMessageID
* Server shutdown message.  Sent by the Application Server when the server is shutting down or restarting.
*
*/
#define SYS_SERVER_SHUTDOWN 20
/**
* \ingroup UDPMessageID
* Find Controller message.  ID property contains the conntrollerID of the sought after controller.  This message is sent as a broadcast message by
* the sending controller which puts its IP address in field[0], field[1], field[2] and field[3] of the message's payload.  If the ID matches the
* receiving controller's controllerID it responds by sending a #SYS_CONTROLLER_ONLINE message to the sender followed by device status messages.
*
*/
#define SYS_FIND_CONTROLLER 21
/**
* \ingroup UDPMessageID
* Reset Controller Notification List message.  ID property contains the serialNumber of the target controller or 0.   When the ID property is set to 0, all
* controllers respond to the message.  The controller will respond by making a http RESTful API call to the server using the url
* /controller/notification_list?serialNumber=xxx where xxx is the controller's serial number.  The server will send back a list of controllerIDs
* the controller must notify when sending commands or status messages.  The list is then saved in the controller's configuration data stored on
* flash memory.
*
*/
#define SYS_RESET_NOTIFICATION_LIST 22
/**
* \ingroup UDPMessageID
* Lock Device message.  ID property contains the deviceID of the device that should be (un)locked.  Field[0] contains a 1 or 0 indicating the device should
* lock or unlock iteslf.  Currently, Turnout, Signal and Panel Input devices respond to this command.
*
*/
#define SYS_LOCK_DEVICE 23
/**
* \ingroup UDPMessageID
* Lock Route message.  ID property contains the routeID of the route that should be (un)locked.  Field[0] contains a 1 or 0 indicating the route should
* be locked or unlocked respectively.  Turnout devices respond to this message by checking its configuration data to see if the turnout is assigned to the
* specified route.  If so, the turnout sets its state to the state defined in the route configuration and sets its locked flag to on.  Once locked, the
* turnout will ignore all #TRN_ACTIVATE and #TRN_ACTIVATE_ROUTE commands until it receives a corresponding #SYS_LOCK_ROUTE command with its field[0] set to 0 (unlock).
*/
#define SYS_LOCK_ROUTE 24

/**@}*/
