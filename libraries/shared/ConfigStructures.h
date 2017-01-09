#pragma once

#include "GlobalDefs.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Turnout Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////

struct TurnoutRouteEntryStruct
{
	int routeID;
	TurnoutState state;
};
typedef struct TurnoutRouteEntryStruct TurnoutRouteEntryStruct;

struct TurnoutConfigStruct
{
	int turnoutID;
	TurnoutRouteEntryStruct routeEntries[MAX_ROUTE_ENTRIES];
};
typedef struct TurnoutConfigStruct TurnoutConfigStruct;


struct TurnoutControllerConfigStruct
{
	TurnoutConfigStruct turnout1;
	TurnoutConfigStruct turnout2;
};
typedef TurnoutControllerConfigStruct TurnoutControllerConfigStruct;
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Panel Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////

struct PanelInputStruct
{
	int id;
	unsigned char inputType;
	unsigned char value;
};
typedef struct PanelInputStruct PanelInputStruct;

struct PanelOutputStruct
{
	int itemID;
	unsigned char itemType;
	unsigned char onValue;
	unsigned char flashingValue;
};
typedef struct PanelOutputStruct PanelOutputStruct;

struct ModuleConfigStruct
{
	PanelOutputStruct outputs[MAX_PANEL_OUTPUTS];
	PanelInputStruct inputs[MAX_PANEL_INPUTS];
};
typedef struct ModuleConfigStruct ModuleConfigStruct;

struct PanelRouteEntryStruct
{
	int turnoutID;
	TurnoutState state;
};
typedef PanelRouteEntryStruct PaneRouteEntryStruct;

struct PanelRouteStruct
{
	int routeID;
	PanelRouteEntryStruct entries[MAX_ROUTE_ENTRIES];
};
typedef PanelRouteStruct PanelRouteStruct;

struct PanelControllerConfigStruct
{
	unsigned char mdouleCount;
	ModuleConfigStruct moduleConfigs[MAX_PANEL_MODULES];
};
typedef struct PanelControllerConfigStruct PanelControllerConfigStruct;

struct PanelControllerRouteConfigStruct
{
	unsigned char count;
	PanelRouteStruct routes[MAX_MODULE_ROUTES];
};
typedef struct PanelControllerRouteConfigStruct PanelControllerRouteConfigStruct;
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Signal Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////
struct SignalAspectStruct
{
        unsigned char redMode;
        unsigned char yellowMode;
        unsigned char greenMode;
};
typedef struct SignalAspectStruct SignalAspectStruct;

struct SignalConditionStruct
{
	int deviceID;
        unsigned char operand;
        unsigned char deviceState;
};
typedef struct SignalConditionStruct SignalConditionStruct;

struct SignalAspectConditionStruct
{
	SignalConditionStruct conditions[MAX_SIGNAL_CONDITIONS];
	SignalAspectStruct aspect;
};
typedef struct SignalAspectConditionStruct SignalAspectConditionStruct;

struct SignalConfigStruct
{
	int signalID;
	SignalAspectConditionStruct conditions[MAX_SIGNAL_CONDITIONS];
};
typedef struct SignalConfigStruct SignalConfigStruct;

struct SignalControllerConfigStruct
{
	SignalConfigStruct signal1;
	SignalConfigStruct signal2;
};
typedef struct SignalControllerConfigStruct SignalControllerConfigStruct;
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Block Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////
struct BlockConfigStruct
{
        short blockID;
};
typedef struct BlockConfigStruct BlockConfigStruct;

struct BlockControllerConfigStruct
{
	BlockConfigStruct block1;
	BlockConfigStruct block2;
};
typedef struct BlockControllerConfigStruct BlockControllerConfigStruct;
