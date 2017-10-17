#pragma once
#include "Local.h"

const int ControllerVersion = 13;

#ifdef PROJECT_DEBUG
#define DEBUG_PRINT(...) Serial.printf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) // nothing to do here
#endif

#define MAX_TURNOUTS 2
#define MAX_BLOCKS 2
#define MAX_SIGNALS 3
#define MAX_MODULES 8
#define MAX_PANEL_MODULES 8
#define MAX_PANEL_OUTPUTS 8
#define MAX_PANEL_INPUTS 8
#define MAX_SIGNAL_PINS 8

#define TIMEOUT_INTERVAL 200
#define HEARTBEAT_INTERVAL 60000 // One minute

enum ControllerStatus
{
    ControllerUnknown,
    ControllerOffline,
    ControllerOnline,
    ControllerRestarting
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

enum ClassEnum
{
        ClassUnknown,
        ClassTurnout,
        ClassPanel,
        ClassRoute,
        ClassSignal,
        ClassSemaphore,
        ClassBlock,
        ClassMulti,
        ClassSystem,
        ClassApp
};

static unsigned char UdpBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF};
const unsigned int UdpPort = 45457;
const unsigned int LocalServerPort = 45455;
const unsigned int sendTimeout = 100; 

// config addresses for EEPROM
#define CONTROLLER_ID_ADDRESS 7

//Message IDS
//System Messages
#define SYS_FIND_SERVER 11
#define SYS_SERVER_HEARTBEAT 12
#define SYS_RESET_CONFIG 13
#define SYS_REBOOT_CONTROLLER 14
#define SYS_DOWNLOAD_FIRMWARE 15
#define SYS_RESTARTING 16
#define SYS_UDP_HTTP 17
#define SYS_KEEP_ALIVE 18
