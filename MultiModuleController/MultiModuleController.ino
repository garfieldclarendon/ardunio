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
#include "InputModule.h"
#include "OutputModule.h"

#define BASE_ADDRESS 0x20  // MCP23008 is on I2C port 0x20

const int MODULE_CONFIG_BASE_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

// STATUS_LED_PIN is used to show the status of the connection to the LCServer
// LED on indicates the controller is connected to the server
// LED off indicates the controller is offline
const byte STATUS_LED_PIN = 2;// use the built in blue LED on the ESP8266 connected to pin2
const byte LOCKOUT_PIN = 13;
bool lockout = false;
byte lastLockoutRead = 0;
long currentLockoutTimeout = 0;

byte totalModules = 0;
bool downloadConfigFlag = false;

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

	loadConfiguration();

	controller.setup((ClassEnum)controllerConfig.controllerClass);

	Network.setModuleConfigCallback(netModuleConfigCallback);
	Network.setControllerCallback(netControllerCallback);
	Network.setControllerConfigCallback(netControllerConfigCallback);
	Network.setModuleCallback(netModuleCallback);
	Network.setUdpMessageCallback(udpMessageCallback);
	Network.setServerConnectedCallback(serverReconnected);

	if (totalModules > 0)
	{
		setupHardware();
		createModules();
		loadModuleConfiguration();

		for (byte x = 0; x < totalModules; x++)
		{
			if (controllerConfig.controllerClass == ClassMulti)
			{
				DEBUG_PRINT("CALLING SETUPWIRE FOR ADDRESS: %d\n", x);
				modules[x]->setupWire(controllerConfig.moduleConfigs[x].address);
			}
			else
			{
				DEBUG_PRINT("CALLING SETUP FOR ADDRESS: %d\n", x);
				modules[x]->setup();
			}
		}
	}

	DEBUG_PRINT("setup complete\n");
}

void serverReconnected(void)
{
	DEBUG_PRINT("serverReconnected\n");

	if (downloadConfigFlag)
	{
		downloadConfig();
	}
	else
	{
		for (byte x = 0; x < totalModules; x++)
		{
			modules[x]->serverOnline();
			modules[x]->sendStatusMessage();
		}
	}
}

void netModuleConfigCallback(NetActionType action, byte address, const JsonObject &json)
{
	DEBUG_PRINT("netConfigCallback: NetAction %d\n", action);

	if (action == NetActionUpdate)
		saveConfig(address, json);
	else if (action == NetActionDelete)
		deleteConfig(address);

	modules[address]->netModuleConfigCallback(action, address, json);
}

void saveConfig(byte address, const JsonObject &json)
{
	String txt;
	json.printTo(txt);
	String fileName("/Module_");
	fileName += address;
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

void deleteConfig(byte address)
{
	String fileName("/Module_");
	fileName += address;
	fileName += ".json";
	SPIFFS.remove(fileName);
}

String netControllerConfigCallback(NetActionType action, const JsonObject &json)
{
	DEBUG_PRINT("netControllerConfigCallback: NetAction %d\n", action);

	controllerConfig.controllerClass = (ClassEnum)(int)json["controllerClass"];

	JsonArray &mods = json["modules"];
	controllerConfig.controllerClass = json["controllerClass"];
	controllerConfig.mdouleCount = mods.size();
	for (byte x = 0; x < mods.size(); x++)
	{
		int index = mods[x]["index"];
		controllerConfig.moduleConfigs[x].moduleClass = mods[x]["class"];
		controllerConfig.moduleConfigs[x].address = mods[x]["address"];
		DEBUG_PRINT("Module Config  Class: %d  Address: %d\n", controllerConfig.moduleConfigs[index].moduleClass, controllerConfig.moduleConfigs[index].address);
	}

	saveControllerConfig();

	controller.restart();

	String ret;
	return ret;
}

void netModuleCallback(NetActionType action, byte address, const JsonObject &json)
{
	if (address < totalModules && address >= 0)
	{
		if (controllerConfig.controllerClass == ClassMulti)
			modules[address]->netModuleCallbackWire(action, address, json);
		else
			modules[address]->netModuleCallbackNoWire(action, address, json);
	}
}

String netControllerCallback(NetActionType action, const JsonObject &json)
{
	controller.controllerCallback(action, json);
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
	if (controllerConfig.controllerClass == ClassMulti)
	{
		Wire.begin(4, 5); //creates a Wire object

		//Pin 12 is the network status indicator LED
		// ON - Connected to the server
		// OFF - Disconnected
		pinMode(STATUS_LED_PIN, OUTPUT);
		digitalWrite(STATUS_LED_PIN, HIGH);
		//Pin 13 is the lockout switch input
		pinMode(LOCKOUT_PIN, INPUT);
	}
}

void loop() 
{
	checkLockoutPin();
	Network.process();
	controller.process();

	if (Network.getIsConnected() == false)
	{
		for (byte x = 0; x < totalModules; x++)
		{
			modules[x]->serverOffline();
		}
	}
	for (byte x = 0; x < totalModules; x++)
	{
		if (controllerConfig.controllerClass == ClassMulti)
			modules[x]->processWire();
		else
			modules[x]->processNoWire();
		yield();
	}

	if (controllerConfig.controllerClass == ClassMulti)
		setStatusIndicator();

	if (totalModules == 0)
		delay(100);
}

void loadConfiguration(void)
{
	if (EEPROM.read(0) == 0xBB)
	{
		// Get the number of modules connected to this controller.
		DEBUG_PRINT("Getting the controller configuration: \n");
		EEPROM.get(MODULE_CONFIG_BASE_ADDRESS, controllerConfig);
		DEBUG_PRINT("Controller coinfiguration complete.  Total modules: %d\n", controllerConfig.mdouleCount);

		totalModules = controllerConfig.mdouleCount;

		DEBUG_PRINT("DONE GETTING CONFIGURATION\n");
	}
	else
	{
		DEBUG_PRINT("EEPROM not set.  Initializing to 0\n");
		// This controller has not been configured yet or the configuration is no longer valid, 
		// write default values to memory

		totalModules = 0;
		memset(&controllerConfig, 0, sizeof(MultiModuleControllerConfigStruct));
		saveControllerConfig();
		DEBUG_PRINT("DONE SAVING EEPROM.  Initializing to 0\n");
		downloadConfigFlag = true;
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

		switch (controllerConfig.moduleConfigs[index].moduleClass)
		{
			case ClassTurnout:
			{
				DEBUG_PRINT("CreateModules:  Creating Turnout Module\n");
				TurnoutModule *turnout = new TurnoutModule();
				modules[index] = turnout;
				break;
			}
			case ClassBlock:
			case ClassInput:
			{
				DEBUG_PRINT("CreateModules:  Creating Input Module\n");
				InputModule *inputMod = new InputModule();
				modules[index] = inputMod;
				break;
			}
			case ClassSignal:
			case ClassOutput:
			{
				DEBUG_PRINT("CreateModules:  Creating Output Module\n");
				OutputModule *outputMod = new OutputModule();
				modules[index] = outputMod;
				break;
			}
			default:
			{
				DEBUG_PRINT("CreateModules:  INVALID MODULE CLASS: %d\n", controllerConfig.moduleConfigs[index].moduleClass);
				allModulesCreated = false;
				totalModules = 0;
				break;
			}
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

void downloadModuelConfig(byte address, ClassEnum classCode)
{
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject &out = jsonBuffer.createObject();
	out["messageUri"] = "/controller/module/config";
	out["address"] = address;
	out["class"] = (int)classCode;
	out["action"] = (int)NetActionGet;

	Network.sendMessageToServer(out);
}

void saveControllerConfig(void)
{
	DEBUG_PRINT("Saving module configuration to EEPROM!!!!!!!\nTotal Modules: %d\n", controllerConfig.mdouleCount);
	
	EEPROM.write(0, 0xBB);
	EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, controllerConfig);
	EEPROM.commit();
}

void checkLockoutPin(void)
{
	long t = millis();
	byte current = digitalRead(LOCKOUT_PIN);
	if (current == lastLockoutRead)
	{
		if (current != lockout && (t - currentLockoutTimeout) > 250)
		{
			currentLockoutTimeout = t;
			lockout = current;
			if (lockout == LOW)
			{
				for (int x = 0; x < totalModules; x++)
				{
					modules[x]->controllerLockout(lockout);
				}
			}
		}
	}
	else
	{
		currentLockoutTimeout = t;
	}
	lastLockoutRead = current;
}

void setStatusIndicator(void)
{
	if (Network.getIsConnected())
		digitalWrite(STATUS_LED_PIN, LOW); // Turn LED on
	else
		digitalWrite(STATUS_LED_PIN, HIGH); // Turn LED off
}