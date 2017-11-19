#pragma once

#include "GlobalDefs.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Controller Configuration Structures
/////////////////////////////////////////////////////////////////////////////////////////

struct DeviceConfigStruct
{
	unsigned char deviceClass;
	byte address;
	byte port;
};
typedef struct DeviceConfigStruct DeviceConfigStruct;

struct ControllerConfigStruct
{
	unsigned char deviceCount;
	DeviceConfigStruct deviceConfigs[MAX_DEVICES];
	unsigned char controllerClass;
};
typedef struct ControllerConfigStruct ControllerConfigStruct;
