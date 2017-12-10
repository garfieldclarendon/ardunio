#pragma once
#include "Local.h"

const unsigned char MajorVersion = 3;
const unsigned char MinorVersion = 0;
const unsigned char ControllerVersion = 14;

#ifdef PROJECT_DEBUG
#define DEBUG_PRINT(...) Serial.printf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) // nothing to do here
#endif

#define MAX_TURNOUTS 2
#define MAX_BLOCKS 2
#define MAX_SIGNALS 3
#define MAX_MODULES 8
#define MAX_DEVICES 16
#define MAX_ROUTE_ENTRIES 5
#define MAX_SIGNAL_DEVICES 10
#define MAX_SIGNAL_CONDITIONS 5

#define TIMEOUT_INTERVAL 200
#define HEARTBEAT_INTERVAL 60000 // One minute

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
    ControllerMulti = 7,
    ControllerTurnout = 1,
    ControllerSemaphore = 5,
    ControllerServer = 10,
    ControllerApp
};

enum DeviceClassEnum
{
    DeviceUnknown,
    DeviceTurnout,
    DevicePanelInput,
    DevicePanelOutput,
    DeviceSignal = 4,
    DeviceSemaphore = 5,
    DeviceBlock = 6,
    DevicePanel = 7 // For old versions--no longer used
};

enum ModuleClassEnum
{
    ModuleUnknown,
    ModuleTurnout = 1,
    ModuleSemaphore = 5,
    ModuleInput = 8,
    ModuleOutput = 9
};

const unsigned char UdpBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF};
const unsigned int UdpPort = 45457;
const unsigned int LocalServerPort = 45455;
const unsigned int sendTimeout = 100;

// config addresses for EEPROM
#define CONTROLLER_ID_ADDRESS 7

// Status Message IDS
#define TRN_STATUS 1
#define BLK_STATUS 2

#define END_STATUS_MESSAGES 5
#define TRN_ACTIVATE 6
#define TRN_ACTIVATE_ROUTE 7

//System Messages
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
