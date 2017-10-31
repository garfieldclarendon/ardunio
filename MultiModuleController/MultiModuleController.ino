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
#include "BlockModule.h"
#include "SignalModule.h"
#include "InputModule.h"

#define BASE_ADDRESS 0x20  // MCP23008 is on I2C port 0x20

const int MODULE_CONFIG_BASE_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

const byte STATUS_LED_PIN = 12;
const byte LOCKOUT_PIN = 13;
bool lockout = false;
byte lastLockoutRead = 0;
long currentLockoutTimeout = 0;

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
	Network.setModuleCallback(netModuleCallback);
	Network.setUdpMessageCallback(udpMessageCallback);
	Network.setServerConnectedCallback(serverReconnected);

	loadConfiguration();
	setupHardware();

	for (byte x = 0; x < totalModules; x++)
	{
		DEBUG_PRINT("CALLING SETUPWIRE FOR ADDRESS: %d\n", x);
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
		modules[x]->serverOnline();
		modules[x]->sendStatusMessage();
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

void netModuleCallback(NetActionType action, byte address, const JsonObject &json)
{
	if (address < totalModules && address >= 0)
	{
		modules[address]->netModuleCallbackWire(action, address, json);
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
	Wire.begin(4, 5); //creates a Wire object

	//Pin 12 is the network status indicator LED
	// ON - Connected to the server
	// OFF - Disconnected
	pinMode(STATUS_LED_PIN, OUTPUT);
	//Pin 13 is the lockout switch input
	pinMode(LOCKOUT_PIN, INPUT);
}

void loop() 
{
	checkLockoutPin();
	Network.process();
	controller.process();

	if (Network.getIsConnected())
	{
		for (byte x = 0; x < totalModules; x++)
		{
			modules[x]->serverOffline();
		}
	}
	bool sendStatus = false;
	for (byte x = 0; x < totalModules; x++)
	{
		modules[x]->processWire();
		yield();
	}
	setStatusIndicator();
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
		else if (controllerConfig.moduleConfigs[index].moduleClass == ClassBlock)
		{
		  DEBUG_PRINT("CreateModules:  Creating Block Module\n");
		  BlockModule *block = new BlockModule();
		  modules[index] = block;
		}
		else if (controllerConfig.moduleConfigs[index].moduleClass == ClassSignal)
		{
		  DEBUG_PRINT("CreateModules:  Creating Signal Module\n");
		  SignalModule *signalMod = new SignalModule();
		  modules[index] = signalMod;
		}
		else if (controllerConfig.moduleConfigs[index].moduleClass == ClassInput)
		{
			DEBUG_PRINT("CreateModules:  Creating Input Module\n");
			InputModuleClass *inputMod = new InputModuleClass();
			modules[index] = inputMod;
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
	
	EEPROM.write(0, 0xAD);
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
			for (int x = 0; x < totalModules; x++)
			{
				modules[x]->controllerLockout(lockout);
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
		digitalWrite(STATUS_LED_PIN, HIGH);
	else
		digitalWrite(STATUS_LED_PIN, LOW);
}