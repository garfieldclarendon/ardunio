#define DEBUG_WEBSOCKETS
#define DEBUG_ESP_WIFI

#include <dummy.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "Module.h"
#include "Controller.h"
#include "GlobalDefs.h"
#include "configStructures.h"
#include "ConfigDownload.h"
#include "TurnoutModule.h"

#define BASE_ADDRESS 0x20  // MCP23008 is on I2C port 0x20

const int MODULE_CONFIG_BASE_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

byte totalModules = 1;
bool downloadModuleConfig = false;

Module *modules[MAX_MODULES];
Controller controller(LocalServerPort);
MultiModuleControllerConfigStruct controllerConfig;

void setup() 
{
#ifdef PROJECT_DEBUG
	Serial.begin(115200);
#else
	Serial.begin(74880);
	Serial.println();
	Serial.println();
	Serial.println("Starting up in release mode");
	Serial.printf("Panel Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");
Serial.setDebugOutput(true);
	memset(&controllerConfig, 0, sizeof(MultiModuleControllerConfigStruct));
	memset(modules, 0, sizeof(Module *) * MAX_MODULES);

	EEPROM.begin(512);
	bool result = SPIFFS.begin();
	DEBUG_PRINT("SPIFFS opened: %d\n", result);

	controller.setup(ClassMulti);

	Network.setModuleConfigCallback(netModuleConfigCallback);
	Network.setControllerCallback(netControllerCallback);
	Network.setControllerConfigCallback(netControllerConfigCallback);
	Network.setControllerStatusCallback(netControllerStatusCallback);
	Network.setModuleCallback(netModuleCallback);
	Network.setUdpMessageCallback(udpMessageCallback);
	Network.setServerConnectedCallback(serverReconnected);

	loadConfiguration();
	setupHardware();

	for (byte x = 0; x < totalModules; x++)
	{
		modules[x]->setupWire(x);
	}

	DEBUG_PRINT("setup complete\n");
}

void serverReconnected(void)
{
	DEBUG_PRINT("serverReconnected\n");
	if (totalModules == 0)
	{
		downloadConfig();
	}
	else if (downloadModuleConfig)
	{
		downloadModuleConfig = false;
		for (byte x = 0; x < totalModules; x++)
		{
			downloadModuelConfig(x, (ClassEnum)controllerConfig.moduleConfigs[x].moduleClass);
		}
	}

	for (byte x = 0; x < totalModules; x++)
	{
		modules[x]->sendStatusMessage();
	}
}

void netControllerStatusCallback(int &controllerID, String &controllerName, String &controllerType, String &currentStatus)
{
	controllerID = controller.getControllerID();
	controllerName = controller.getControllerName();
	controllerType = "Multi-Module Controller";

	currentStatus = "<table>";
	currentStatus += "<tr>";
	currentStatus += "<th>Device ID</th>";
	currentStatus += "<th>Device Name</th>";
	currentStatus += "</tr>";
	for (byte x = 0; x < controller.getDeviceCount(); x++)
	{
		currentStatus += "<tr>";
		currentStatus += "<td>";
		currentStatus += controller.getDeviceID(x);
		currentStatus += "</td>";
		currentStatus += "<td>";
		currentStatus += controller.getDeviceName(x);
		currentStatus += "</td>";
		currentStatus += "</tr>";
	}
	currentStatus += "</table>";
}

void netModuleConfigCallback(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	DEBUG_PRINT("netConfigCallback: NetAction %d\n", action);

	if (action == NetActionUpdate)
		saveConfig(moduleIndex, json);
	else if (action == NetActionDelete)
		deleteConfig(moduleIndex);

	modules[moduleIndex]->netModuleConfigCallback(action, moduleIndex, json);
}

void saveConfig(byte moduleIndex, const JsonObject &json)
{
	String txt;
	json.printTo(txt);
	String fileName("/Module_");
	fileName += moduleIndex;
	fileName += ".json";

	if (SPIFFS.exists(fileName))
		SPIFFS.remove(fileName);
	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		DEBUG_PRINT("Saving Module config: %s\n", fileName.c_str());

		f.write((const uint8_t *)txt.c_str(), txt.length());
		f.close();
	}
	else
	{
		DEBUG_PRINT("Saving Module config FAILED: %s  COULD NOT OPEN FILE\n", fileName.c_str());
	}
}

void deleteConfig(byte moduleIndex)
{
	String fileName("/Module_");
	fileName += moduleIndex;
	fileName += ".json";
	SPIFFS.remove(fileName);
}

String netControllerConfigCallback(NetActionType action, const JsonObject &json)
{
	DEBUG_PRINT("netControllerConfigCallback: NetAction %d\n", action);

	controllerConfig.extraPin0Mode = (PinModeEnum)(int)json["extrPin0Mode"];
	controllerConfig.extraPin1Mode = (PinModeEnum)(int)json["extrPin1Mode"];
	controllerConfig.extraPin2Mode = (PinModeEnum)(int)json["extrPin2Mode"];
	controllerConfig.extraPin3Mode = (PinModeEnum)(int)json["extrPin3Mode"];
	controllerConfig.extraPin4Mode = (PinModeEnum)(int)json["extrPin4Mode"];

	JsonArray &mods = json["modules"];
	controllerConfig.mdouleCount = mods.size();
	for (byte x = 0; x < mods.size(); x++)
	{
		int index = mods[x]["index"];
		controllerConfig.moduleConfigs[index].moduleClass = (int)mods[x]["class"];
	}

	saveControllerConfig();

	controller.restart();

	String ret;
	return ret;
}

void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	if (moduleIndex < totalModules && moduleIndex >= 0)
	{
		modules[moduleIndex]->netModuleCallbackWire(action, moduleIndex, json);
	}
}

String netControllerCallback(NetActionType action, const JsonObject &json)
{
	controller.controllerCallback(action, json);
	//if (action == NetActionUpdate)
	//{
	//	StaticJsonBuffer<250> jsonBuffer;
	//	JsonObject &root = jsonBuffer.parseObject(json);

	//	if (root.containsKey("extrPin0Mode"))
	//		controller.setExtraPin(0, (PinStateEnum)(int)root["extrPin0Mode"]);
	//	if (root.containsKey("extrPin1Mode"))
	//		controller.setExtraPin(1, (PinStateEnum)(int)root["extrPin1Mode"]);
	//	if (root.containsKey("extrPin2Mode"))
	//		controller.setExtraPin(2, (PinStateEnum)(int)root["extrPin2Mode"]);
	//	if (root.containsKey("extrPin3Mode"))
	//		controller.setExtraPin(3, (PinStateEnum)(int)root["extrPin3Mode"]);
	//	if (root.containsKey("extrPin4Mode"))
	//		controller.setExtraPin(4, (PinStateEnum)(int)root["extrPin4Mode"]);
	//}
	// TODO: Consider returning the current state of all output pins
	String ret;
	return ret;
}

void udpMessageCallback(const Message &message)
{
	controller.processMessage(message);
}

void setupHardware(void)
{
	DEBUG_PRINT("setup hardware\n");
	Wire.begin(4, 5); //creates a Wire object

	//controller.addExtraPin(0, 2, controllerConfig.extraPin0Mode);
	//controller.addExtraPin(1, 0, controllerConfig.extraPin1Mode);
	//controller.addExtraPin(2, 12, controllerConfig.extraPin2Mode);
	//controller.addExtraPin(3, 14, controllerConfig.extraPin3Mode);
	//controller.addExtraPin(4, 16, controllerConfig.extraPin4Mode);
}

void loop() 
{
	Network.process();
	controller.process();

	bool sendStatus = false;
	for (byte x = 0; x < totalModules; x++)
	{
		modules[x]->processWire();
	}
}

void loadConfiguration(void)
{
	if (EEPROM.read(0) == 0xAD)
	{
		// Get the number of modules connected to this controller.
		DEBUG_PRINT("Getting the controller configuration: \n");
		EEPROM.get(MODULE_CONFIG_BASE_ADDRESS, controllerConfig);
		DEBUG_PRINT("Controller coinfiguration complete.  Total modules: %d\n", controllerConfig.mdouleCount);

		totalModules = controllerConfig.mdouleCount;

		DEBUG_PRINT("DONE GETTING CONFIGURATION\n");

		createModules();
		loadModuleConfiguration();
	}
	else
	{
		DEBUG_PRINT("EEPROM not set.  Initializing to 0\n");
		// This controller has not been configured yet or the configuration is no longer valid, 
		// write default values to memory

		totalModules = 0;
		memset(&controllerConfig, 0, sizeof(MultiModuleControllerConfigStruct));
		EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, controllerConfig);
		EEPROM.commit();
		DEBUG_PRINT("DONE SAVING EEPROM.  Initializing to 0\n");
	}
}

void loadModuleConfiguration(void)
{
	DEBUG_PRINT("LOAD CONFIGURATION\n");
	for (byte x = 0; x < totalModules; x++)
	{
		String json;
		String fileName("/Module_");
		fileName += x;
		fileName += ".json";
		File f = SPIFFS.open(fileName, "r");

		if (f)
		{
			DEBUG_PRINT("Reading Module config: %s\n", fileName.c_str());

			json = f.readString();
			f.close();

			if (json.length() > 0)
			{
				StaticJsonBuffer<512> jsonBuffer;
				JsonObject &root = jsonBuffer.parseObject(json);
				modules[x]->netModuleConfigCallback(NetActionUpdate, x, root);
			}
		}
		else
		{
			DEBUG_PRINT("Configuration file %s is missing or can not be opened\n", fileName.c_str());
			downloadModuleConfig = true;
		}
	}
}

void createModules(void)
{
	DEBUG_PRINT("CreateModules\n");
	bool allModulesCreated = true;

	for (byte index = 0; index < totalModules; index++)
	{
		if (modules[index] != NULL)
			delete modules[index];

		if (controllerConfig.moduleConfigs[index].moduleClass == ClassTurnout)
		{
			DEBUG_PRINT("CreateModules:  Creating Turnout Module\n");
			TurnoutModule *turnout = new TurnoutModule();
			modules[index] = turnout;
		}
		else
		{
			allModulesCreated = false;
			totalModules = 0;
		}
	}
}

void downloadConfig(void)
{
	DEBUG_PRINT("downloadConfig\n");

	StaticJsonBuffer<200> jsonBuffer;
	JsonObject &out = jsonBuffer.createObject();

	out["messageUri"] = "/controller/multiConfig";
	out["serialNumber"] = ESP.getChipId();
	out["classCode"] = (int)ClassMulti;
	out["action"] = (int)NetActionGet;

	Network.sendMessageToServer(out);
}

void downloadModuelConfig(byte moduleIndex, ClassEnum classCode)
{
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject &out = jsonBuffer.createObject();
	out["messageUri"] = "/controller/module/config";
	out["moduleIndex"] = moduleIndex;
	out["class"] = (int)classCode;
	out["action"] = (int)NetActionGet;

	Network.sendMessageToServer(out);
}

void saveControllerConfig(void)
{
	DEBUG_PRINT("Saving module configuration to EEPROM!!!!!!!\nTotal Modules: %d\n", controllerConfig.mdouleCount);
	
	EEPROM.write(0, 0xAD);
	EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, controllerConfig);
	EEPROM.commit();
}
