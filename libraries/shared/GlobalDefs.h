#pragma once
//#define PROJECT_DEBUG

const int ControllerVersion = 7;

#ifdef PROJECT_DEBUG
#define DEBUG_PRINT(...) Serial.printf( __VA_ARGS__ )
#else
#define DEBUG_PRINT(...) // nothing to do here
#endif

#define MAX_PANEL_MODULES 8
#define MAX_ROUTE_ENTRIES 5
#define MAX_PANEL_OUTPUTS 8
#define MAX_PANEL_INPUTS 8
#define MAX_MODULE_ROUTES 64
//#define MAX_MODULE_ROUTE_DOWNLOADS 8
#define MAX_SIGNAL_CONDITIONS 10

#define TIMEOUT_INTERVAL 200
#define HEARTBEAT_INTERVAL 60000 // One minute

enum TurnoutState 
{
  TrnUnknown,
  TrnNormal,
  TrnToDiverging,
  TrnDiverging,
  TrnToNormal
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
        ClassSystem,
        ClassApp
};
static unsigned char UdpBroadcast[] = {0xFF, 0xFF, 0xFF, 0xFF};
const int UdpPort = 45454;
const int LocalServerPort = 45455;
const unsigned int sendTimeout = 100; 

struct RouteEntryStruct
{
	int turnoutID;
	TurnoutState state;
};
typedef struct RouteEntryStruct RouteEntryStruct;

struct RouteStruct
{
	int routeID;
	RouteEntryStruct routeEntries[MAX_ROUTE_ENTRIES];
};
typedef struct RouteStruct RouteStruct;

// config addresses for EEPROM
#define CONTROLLER_ID_ADDRESS 7

//Message IDS
//System Messages
#define SYS_SET_CONTROLLER_ID 12
#define SYS_NEW_CONTROLLER 13
#define SYS_HEARTBEAT 14
#define SYS_CONFIG_CHANGED 15
#define SYS_REBOOT_CONTROLLER 16
#define SYS_DOWNLOAD_FIRMWARE 17

//Turnout Messages
#define TRN_STATUS 101
#define TRN_ACTIVATE 102

//Signal Messages
#define SIG_STATUS 121
#define SIG_SET_ASPECT 122

//Block Messages
#define BLOCK_STATUS 201

//Panel Messages
#define PANEL_ACTIVATE_ROUTE 221
#define PANEL_STATUS 222

