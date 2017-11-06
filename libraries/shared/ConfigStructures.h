#pragma once

#include "GlobalDefs.h"


//////////////////////////////////////////////////////////////////////////////////////////
// Multi Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////

struct MultiModuleConfigStruct
{
	unsigned char moduleClass;
	byte address;
};
typedef struct MultiModuleConfigStruct MultiModuleConfigStruct;

struct MultiModuleControllerConfigStruct
{
	unsigned char mdouleCount;
	MultiModuleConfigStruct moduleConfigs[MAX_MODULES];
	unsigned char controllerClass;
};
typedef struct MultiModuleControllerConfigStruct MultiModuleControllerConfigStruct;

//////////////////////////////////////////////////////////////////////////////////////////
// Turnout Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////

struct TurnoutConfigStruct
{
	int deviceID;
	unsigned char inputPinSetting;
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
//////////////////////////////////////////////////////////////////////////////////////////
struct PanelControllerConfigStruct
{
        unsigned char totalModules;
	PinModeEnum extraPin0Mode;
	PinModeEnum extraPin1Mode;
	PinModeEnum extraPin2Mode;
	PinModeEnum extraPin3Mode;
	PinModeEnum extraPin4Mode;
};
typedef PanelControllerConfigStruct PanelControllerConfigStruct;

//////////////////////////////////////////////////////////////////////////////////////////
// Signal Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
// Block Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////
struct BlockConfigStruct
{
        short deviceID;
};
typedef struct BlockConfigStruct BlockConfigStruct;

struct BlockControllerConfigStruct
{
	BlockConfigStruct block1;
	BlockConfigStruct block2;
};
typedef struct BlockControllerConfigStruct BlockControllerConfigStruct;
