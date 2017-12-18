#pragma once
#include "Local.h"
/** @file */

const unsigned char MajorVersion = 3;
const unsigned char MinorVersion = 0;
const unsigned char BuildVersion = 16;

#ifdef PROJECT_DEBUG
#define DEBUG_PRINT(...) Serial.printf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) // nothing to do here
#endif

#define MAX_TURNOUTS 2 // Total turnouts per module
#define MAX_MODULES 8 // Total modules per controller
#define MAX_DEVICES 16 // Total devices per module
#define MAX_ROUTE_ENTRIES 5  // Maximum routes a turnout can belong to
#define MAX_SIGNAL_DEVICES 10 // Maximum devices (turnout and blocks) monitored by a Signal device
#define MAX_SIGNAL_CONDITIONS 5  // Maximum conditions per aspect

#define TIMEOUT_INTERVAL 200 // Input de-bounce timeout interval
#define HEARTBEAT_INTERVAL 60000 // One minute heatbeat broadcast message interval

/*!
    \enum ControllerStatusEnum
    This enum describes the current status of a controller

    \value ControllerStatusUnknown The controller's current status cannot be determined.
    \value ControllerStatusOffline The controller is offline.
    \value ControllerStatusOnline The controller is online (replaces ControllerStatusConnected).
    \value ControllerStatusRestarting The controller is restarting.
    \value ControllerStatusConected No longer used.  Kept for backward compatability.
*/
enum ControllerStatusEnum
{
    ControllerStatusUnknown,
    ControllerStatusOffline,
    ControllerStatusOnline,
    ControllerStatusRestarting,
    ControllerStatusConected
};

enum ConditionEnum
{
    ConditionEquals,
    ConditionNotEquals
};

enum PinModeEnum
{
    PinUnknown,
    PinInput,
    PinInputPullup,
    PinOutput,
    PinOutputOpenDrain
};

enum PinStateEnum
{
    PinOff,
    PinOn,
    PinFlashing
};

enum NetActionType
{
    NetActionGet,
    NetActionAdd,
    NetActionUpdate,
    NetActionDelete
};

enum TurnoutState 
{
  TrnUnknown,
  TrnNormal,
  TrnToDiverging,
  TrnDiverging,
  TrnToNormal
};

enum BlockState
{
    BlockUnknown,
    BlockClear,
    BlockOccupied
};

enum ControllerClassEnum
{
    ControllerUnknown,
    ControllerMulti = 7,        // Multi-Module Controller.  Supports up to 8 devices connected by an I2C serial interface
    ControllerTurnout = 1,      // Turnout Controller which is a Controller + Turnout Module combined on one board
    ControllerSemaphore = 5,    // Semaphore Signal Controller which is a Controller + Turnout module combined on one board
    ControllerServer = 10,      // Application Server either a PC desktop or a Raspberry Pi
    ControllerApp               // Support/Configuration application
};

enum DeviceClassEnum
{
    DeviceUnknown,
    DeviceTurnout,          // Turnout Device
    DevicePanelInput,       // Panel Button Input device
    DevicePanelOutput,      // Panel LED Output Module
    DeviceSignal = 4,       // Signal Device
    DeviceSemaphore = 5,    // Semaphore Signal Device
    DeviceBlock = 6,        // Block Device
    DevicePanel = 7         // For old versions--no longer used
};

enum ModuleClassEnum
{
    ModuleUnknown,
    ModuleTurnout = 1,      // Turnout Module
    ModuleSemaphore = 5,    // Semaphore Module which is really a turnout module
    ModuleInput = 8,        // Input Module used by Panel Input and Block devices
    ModuleOutput = 9        // Output Module used by Panel Output and Signal devices
};

const unsigned char UdpBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF};
const unsigned int UdpPort = 45457;
const unsigned int LocalServerPort = 45455;

// config addresses for EEPROM
#define CONTROLLER_ID_ADDRESS 7

// Status Message IDs
#define TRN_STATUS 1
#define BLK_STATUS 2

#define END_STATUS_MESSAGES 5
#define TRN_ACTIVATE 6
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
