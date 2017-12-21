#pragma once
#include "Local.h"
/** @file */

const unsigned char MajorVersion = 3;
const unsigned char MinorVersion = 0;
const unsigned char BuildVersion = 17;

/** When PROJECT_DEBUG is defined, DEBUG_PRINT will print output to the serial port.  Otherwise, DEBUG_PRINT does nothing (for release builds)  */
#ifdef PROJECT_DEBUG
#define DEBUG_PRINT(...) Serial.printf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) // nothing to do here
#endif

/** Total turnouts per module */
#define MAX_TURNOUTS 2
/** Total modules per controller */
#define MAX_MODULES 8
/** Total devices per module */
#define MAX_DEVICES 16
/** Maximum routes a turnout can belong to */
#define MAX_ROUTE_ENTRIES 5
/** Maximum devices (turnout and blocks) monitored by a Signal device */
#define MAX_SIGNAL_DEVICES 10
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
    ControllerStatusConected ///< No longer used.  Kept for backward compatability.
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
 * \defgroup UDPMessage UDP Message IDs
 * UDP Messages are sent between controllers to execute commands and provide status updates.  Each message is represented by a @ref UDPMessage "UDPMessage"
 * @{
 */
/**
* \ingroup UDPMessage
* Turnout Device status message
*/
#define TRN_STATUS 1
/**
* \ingroup UDPMessage
* Block Device status message
*/
#define BLK_STATUS 2

/**
* \ingroup UDPMessage
* Place holder; used in for loops to determine the upper bound of status message IDs
*/
#define END_STATUS_MESSAGES 5
/**
* \ingroup UDPMessage
* Activate Turnout message.  ID property contains the deviceID of the turnout to activate.  Field0 property contains the TurnoutState (TrnUnknown, TrnNormal or TrnDiverging).
* If Field0 is set to TrnUnknown, the command "toggles" the turnout between Normal and Diverging.
*/
#define TRN_ACTIVATE 6
/**
* \ingroup UDPMessage
* Activate Turnout route message.  ID property contains the routeID.  The Turnout device locates the route in its configuration data and sets the turnout state as defined by the route.
*
*/
#define TRN_ACTIVATE_ROUTE 7

//System Messages
#define SYS_ACK 10
#define SYS_CONTROLLER_ONLINE 11
#define SYS_SERVER_HEARTBEAT 12
#define SYS_RESET_CONFIG 13
#define SYS_REBOOT_CONTROLLER 14
#define SYS_DOWNLOAD_FIRMWARE 15
#define SYS_RESTARTING 16
#define SYS_UDP_HTTP 17
#define SYS_KEEP_ALIVE 18
#define SYS_RESET_DEVICE_CONFIG 19
#define SYS_SERVER_SHUTDOWN 20
#define SYS_FIND_CONTROLLER 21
#define SYS_RESET_NOTIFICATION_LIST 22
#define SYS_LOCK_DEVICE 23
#define SYS_LOCK_ROUTE 24

/**@}*/
