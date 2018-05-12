// Required ESP/Arduino includes
#include <dummy.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

#include "UDPMessage.h"

// Main controller files
#include "Controller.h"
#include "GlobalDefs.h"
#include "NetworkManager.h"

// Modules
#include "TurnoutModule.h"
#include "InputModule.h"
#include "OutputModule.h"

// Devices
#include "TurnoutDevice.h"
#include "PanelOutputDevice.h"
#include "PanelInputDevice.h"
#include "SignalDevice.h"
#include "SemaphoreDevice.h"
#include "BlockDevice.h"

struct DeviceConfigStruct
{
	byte deviceClass;
	byte port;
	int deviceID;
};
typedef struct DeviceConfigStruct DeviceConfigStruct;

struct ModuleConfigStruct
{
	byte moduleClass;
	byte address;
	byte deviceCount;
	DeviceConfigStruct devices[MAX_DEVICES];
};
typedef struct ModuleConfigStruct ModuleConfigStruct;

struct ControllerConfigStruct
{
	ModuleConfigStruct modules[MAX_MODULES];
};
typedef struct ControllerConfigStruct ControllerConfigStruct;

// STATUS_LED_PIN is used to show the status of the connection to the WiFi network
// LED on indicates the controller is connected
// LED off indicates the controller is offline
const byte STATUS_LED_PIN = 2;// use the built in blue LED on the ESP8266 connected to pin2
const byte LOCKOUT_PIN = 13;
const byte CONFIG_VERSION = 1;
bool lockout = false;
byte lastLockoutRead = 0;
long currentLockoutTimeout = 0;

bool downloadConfigFlag = false;

ControllerClassEnum controllerClass = ControllerUnknown;
int controllerID = 0;
byte moduleCount = 0;

Controller controller(LocalServerPort);
String jsonTextToSave;
int nextModuleToNotify = -1;
Module *modules[MAX_MODULES];
StaticJsonBuffer<2048> jsonBuffer;

void setup() 
{
	Serial.begin(115200);
#ifdef PROJECT_DEBUG
	Serial.println("-------------------------------------------------");
	Serial.printf("LCS Controller Version: %d.%d.%d\n\n", MajorVersion, MinorVersion, BuildVersion);
	Serial.print("DEBUG BUILD STARTING.  Available RAM: ");
	Serial.println(ESP.getFreeHeap());
	Serial.println("-------------------------------------------------");
#else
	Serial.println();
	Serial.println();
	Serial.println("-------------------------------------------------");
	Serial.printf("LCS Controller Version: %d.%d.%d\n\n", MajorVersion, MinorVersion, BuildVersion);
	Serial.print("Available RAM: ");
	Serial.println(ESP.getFreeHeap());
	Serial.printf("Serial Number: %d\n\n", ESP.getChipId());
	Serial.println("-------------------------------------------------");
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");
	EEPROM.begin(128);
	DEBUG_PRINT("SPIFFS SIZE %d REAL SIZE %d  ID %d\n", ESP.getFlashChipSize(), ESP.getFlashChipRealSize(), ESP.getFlashChipId());
	bool result = SPIFFS.begin();
	DEBUG_PRINT("SPIFFS opened: %d\n", result);
	
	memset(modules, 0, sizeof(Module*)*MAX_MODULES);
	ControllerConfigStruct controllerConfig;
	memset(&controllerConfig, 0, sizeof(ControllerConfigStruct));

	loadControllerConfiguration(&controllerConfig);

	setupHardware();

	if (downloadConfigFlag == false)
		createModules(&controllerConfig);

	controller.setServerFoundCallback(serverFound);
	controller.setSendStatusCallback(sendStatusMessage);
	controller.setup((ControllerClassEnum)controllerClass, controllerID);

	for (byte x = 0; x <moduleCount; x++)
	{
		Module *module = modules[x];

		if (module)
		{
			if (controllerClass == ControllerMulti)
			{
				module->finishSetupWire();
			}
			else
			{
				module->finishSetupNoWire();
			}
		}
	}

	NetManager.setUdpMessageCallback(udpMessageCallback);
	NetManager.setWIFIConnectCallback(networkConnected);
	NetManager.setNotificationListChangedCallback(notificationListChanged);
	NetManager.init(8080, controllerID);

	DEBUG_PRINT("setup complete\n");
}
void serverFound(void)
{
	DEBUG_PRINT("serverFound\n");

	if (downloadConfigFlag)
	{
		downloadConfig();
	}
	for (byte x = 0; x < moduleCount; x++)
	{
		Module *module = modules[x];
		if (module)
		{
			UDPMessage outMessage;
			outMessage.setMessageID(DEVICE_STATUS);
			outMessage.setID(module->getAddress());
			byte count = 0;
			for (byte index = 0; index < MAX_DEVICES; index++)
			{
				if (count >= 8)
				{
					NetManager.sendUdpMessage(outMessage, true);
					outMessage = UDPMessage();
					outMessage.setMessageID(DEVICE_STATUS);
					outMessage.setID(module->getAddress());
					count = 0;
				}
				Device *device = module->getDevice(index);
				if (device)
				{
					device->serverFound(outMessage, count);
				}
			}
			if (count > 0)
			{
				NetManager.sendUdpMessage(outMessage, true);
			}
		}
	}
	DEBUG_PRINT("serverFound COMPLETE\n");
}

void networkConnected(bool connected)
{
	DEBUG_PRINT("networkConnected: %s\n", connected ? "CONNECTED" : "DISCONNECTED");
	if (connected)
		controller.networkOnline();
	else
		controller.networkOffline();

	for (byte x = 0; x < moduleCount; x++)
	{
		Module *module = modules[x];
		if (module)
		{
			for (byte index = 0; index < MAX_DEVICES; index++)
			{
				Device *device = module->getDevice(index);
				if (device)
				{
					if (connected)
						device->networkOnline();
					else
						device->networkOffline();
				}
			}
			module->sendStatusMessage();
		}
	}
}

void udpMessageCallback(const UDPMessage &message)
{
	DEBUG_PRINT("udpMessageCallback: %d\n", message.getMessageID());
	controller.processMessage(message);
	for (byte x = 0; x < moduleCount; x++)
	{
		Module *module = modules[x];
		if (module)
		{
			if (controllerClass == ControllerMulti)
				module->processUDPMessageWire(message);
			else
				module->processUDPMessageNoWire(message);
		}
	}
}

void setupHardware(void)
{
	DEBUG_PRINT("Setup Hardware: %d\n", controllerClass);
	if (controllerClass == ControllerMulti)
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
	NetManager.process();
	controller.process();

	for (byte x = 0; x < moduleCount; x++)
	{
		if (controllerClass == ControllerMulti)
		{
			if (modules[x])
				modules[x]->processWire();
		}
		else
		{
			if (modules[x])
				modules[x]->processNoWire();
		}
	}

	if (controllerClass == ControllerMulti)
		setStatusIndicator();

	if (jsonTextToSave.length() > 0)
	{
		saveControllerConfig(jsonTextToSave, false);
		jsonTextToSave = "";
	}

	if (nextModuleToNotify >= 0)
	{
		Module *m = modules[nextModuleToNotify++];
		if (m)
			m->sendStatusMessage();
		if (nextModuleToNotify >= moduleCount)
			nextModuleToNotify = -1;
	}
	if (moduleCount == 0)
		delay(100);
	else
		delay(50);
}

String getConfiguration(void)
{
	DEBUG_PRINT("GET CONTROLLER CONFIGURATION\n");
	String json;
	String fileName("/ControllerConfig.json");
	File f = SPIFFS.open(fileName, "r");

	if (f)
	{
		json = f.readString();
		DEBUG_PRINT("Configuration  LENGTH: %d:\n%s\n", json.length(), json.c_str());
		f.close();
	}
	else
	{
		DEBUG_PRINT("Configuration file %s is missing or can not be opened.  Initializing to 0\n", fileName.c_str());
	}
	return json;
}

void loadControllerConfiguration(struct ControllerConfigStruct *config)
{
	DEBUG_PRINT("LOAD CONTROLLER CONFIGURATION\n");
	String json(getConfiguration());

	if (json.length() > 0)
	{
		if (parseControllerConfig(json, true, config) == false)
		{
			DEBUG_PRINT("SETTING DOWLOADFLAG TO TRUE\n");
			downloadConfigFlag = true;
		}
	}
	else
	{
		DEBUG_PRINT("SETTING DOWLOADFLAG TO TRUE\n");
		downloadConfigFlag = true;
	}
}

void loadModuleConfiguration(byte address, struct ModuleConfigStruct *config)
{
	DEBUG_PRINT("LOAD MODULE CONFIGURATION.  ADDRESS: %d\n", address);
	String json;
	String fileName("/ModuleConfig_");
	fileName += address;
	fileName += ".json";
	File f = SPIFFS.open(fileName, "r");

	if (f)
	{
		json = f.readString();
		DEBUG_PRINT("Configuration:\n%s\n", json.c_str());
		f.close();
	}
	else
	{
		DEBUG_PRINT("Configuration file %s is missing or can not be opened.  Initializing to 0\n", fileName.c_str());
	}

	if (json.length() > 0)
	{
		parseModuleConfig(json, config);
	}
	else
	{
//		downloadConfigFlag = true;
	}
}

void createDevices(struct ModuleConfigStruct *moduleConfig, Module *module)
{
	DEBUG_PRINT("CreateDevices.  Device Count: %d\n", moduleConfig->deviceCount);

	if (module)
	{
		for (byte x = 0; x < moduleConfig->deviceCount; x++)
		{
			Device *device = NULL;
			switch (moduleConfig->devices[x].deviceClass)
			{
				case DeviceTurnout:
				{
					DEBUG_PRINT("CreateDevices:  Creating Turnout Device\n");
					device = new TurnoutDevice();
					break;
				}
				case DeviceBlock:
				{
					DEBUG_PRINT("CreateDevices:  Creating Block Device\n");
					device = new BlockDevice();
					break;
				}
				case DeviceSignal:
				{
					DEBUG_PRINT("CreateDevices:  Creating Signal Device\n");
					device = new SignalDevice();
					break;
				}
				case DeviceSemaphore:
				{
					DEBUG_PRINT("CreateDevices:  Creating Signal Device\n");
					device = new SemaphoreDevice();
					break;
				}
				case DevicePanelOutput:
				{
					DEBUG_PRINT("CreateDevices:  Creating PanelOutput Device\n");
					device = new PanelOutputDevice();
					break;
				}
				case DevicePanelInput:
				{
					DEBUG_PRINT("CreateDevices:  Creating PanelInput Device\n");
					device = new PanelInputDevice();
					break;
				}
				default:
				{
					DEBUG_PRINT("CreateDevices:  INVALID DEVICE CLASS: %d\n", moduleConfig->devices[x].deviceClass);
					break;
				}
			}
			if (device)
			{
				device->setup(moduleConfig->devices[x].deviceID, moduleConfig->devices[x].port);
				module->addDevice(x, device);
			}
		}
	}
}

void createModules(struct ControllerConfigStruct *controllerConfig)
{
	DEBUG_PRINT("CreateModules. Count: %d\n", moduleCount);
	for (byte x = 0; x < moduleCount; x++)
	{
		Module *module = NULL;
		loadModuleConfiguration(controllerConfig->modules[x].address, &controllerConfig->modules[x]);
		switch (controllerConfig->modules[x].moduleClass)
		{
			case ModuleSemaphore:
			case ModuleTurnout:
			{
				DEBUG_PRINT("createModules:  Creating Turnout Module address: %d\n", controllerConfig->modules[x].address);
				module = new TurnoutModule;
				break;
			}
			case ModuleInput:
			{
				DEBUG_PRINT("createModules:  Creating Input Module address: %d\n", controllerConfig->modules[x].address);
				module = new InputModule;
				break;
			}
			case ModuleOutput:
			{
				DEBUG_PRINT("createModules:  Creating Output Module address: %d\n", controllerConfig->modules[x].address);
				module = new OutputModule;
				break;
			}
			default:
			{
				DEBUG_PRINT("createModules:  INVALID MODULE CLASS: %d\n", controllerConfig->modules[x].moduleClass);
				break;
			}
		}
		modules[x] = module;
		if (module)
		{
			if (controllerClass == ControllerMulti)
				module->setupWire(controllerConfig->modules[x].address);
			else
				module->setupNoWire();
			createDevices(&controllerConfig->modules[x], module);
		}
	}
}

void downloadConfig(void)
{
	DEBUG_PRINT("downloadConfig\n");

	String payload = NetManager.getControllerConfig(ESP.getChipId());
	if (payload.length() > 0)
	{
		ControllerConfigStruct config;
		memset(&config, 0, sizeof(ControllerConfigStruct));
		if (parseControllerConfig(payload, false, &config))
		{
			downloadModuleConfigs(&config);
			downloadConfigFlag = false;
			saveControllerConfig(payload, true);
			controller.restart();
		}
	}
}

void downloadModuleConfigs(ControllerConfigStruct *config)
{
	for (byte x = 0; x < moduleCount; x++)
	{
		String json = NetManager.getModuleConfig(ESP.getChipId(), config->modules[x].address);
		if (json.length() > 0)
		{
			saveModuleConfig(json, config->modules[x].address);
		}
	}
}

bool parseControllerConfig(const String &jsonText, bool checkVersion, ControllerConfigStruct *config)
{
	DEBUG_PRINT("parseControllerConfig\n");
	jsonBuffer.clear();
	JsonObject &json = jsonBuffer.parseObject(jsonText);

	if (checkVersion && json["version"] != (int)CONFIG_VERSION)
	{
		DEBUG_PRINT("parseControllerConfig  WRONG VERSION.\n");
		return false;
	}

	controllerClass = (ControllerClassEnum)(int)json["controllerClass"];
	controllerID = json["controllerID"];

	if (controllerClass == ControllerUnknown)
	{
		DEBUG_PRINT("parseControllerConfig  INVALID CONTROLLER CLASS.\n");
		return false;
	}

	JsonArray &modules = json["modules"];
	moduleCount = modules.size();
	for (byte x = 0; x < modules.size(); x++)
	{
		JsonArray &devices = modules[x]["devices"];
		config->modules[x].moduleClass = modules[x]["class"];
		config->modules[x].address = modules[x]["address"];
	}

	JsonArray &notifications = json["controllersToNotify"];
	DEBUG_PRINT("parseControllerConfig:  Notification List size: %d\n", notifications.size());
	for (byte x = 0; x < notifications.size(); x++)
	{
		int id = notifications[x]["controllerID"];
		if (controllerID != id)
			NetManager.addNotificationController(id);
	}

	return true;
}

void parseModuleConfig(const String &jsonText, ModuleConfigStruct *config)
{
	DEBUG_PRINT("parseModuleConfig\n");
	jsonBuffer.clear();
	JsonObject &json = jsonBuffer.parseObject(jsonText);

	JsonArray &devices = json["devices"];
	config->deviceCount = devices.size();
	for (byte x = 0; x < devices.size(); x++)
	{
		config->devices[x].port = devices[x]["p"];
		config->devices[x].deviceClass = devices[x]["c"];
		config->devices[x].deviceID = devices[x]["id"];

		DEBUG_PRINT("Device Config  Class: %d  Port: %d\n", config->devices[x].deviceClass, config->devices[x].port);
	}
}

void saveControllerConfig(const String &jsonText, bool addVersion)
{
	DEBUG_PRINT("Saving controller configuration to file.  Total Modules: %d\n", moduleCount);
	String s;
	if (addVersion)
	{
		jsonBuffer.clear();
		JsonObject &json = jsonBuffer.parseObject(jsonText);

		json["version"] = (int)CONFIG_VERSION;
		json.printTo(s);
	}
	else
	{
		s = jsonText;
	}

	String fileName("/ControllerConfig.json");

	if (SPIFFS.exists(fileName))
		SPIFFS.remove(fileName);
	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		DEBUG_PRINT("Saving Controller config: %s\n", fileName.c_str());

//		DEBUG_PRINT("Updated Configuration with version:\n%s\n", s.c_str());
		f.write((const uint8_t *)s.c_str(), s.length());
		f.close();
	}
	else
	{
		DEBUG_PRINT("Saving controller config FAILED: %s  COULD NOT OPEN FILE\n", fileName.c_str());
	}
}

void saveModuleConfig(const String &jsonText, byte address)
{
	DEBUG_PRINT("Saving module configuration to file.\n");

	String fileName("/ModuleConfig_");
	fileName += address;
	fileName +=".json";

	if (SPIFFS.exists(fileName))
		SPIFFS.remove(fileName);
	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		DEBUG_PRINT("Saving module config: %s\n", fileName.c_str());

		//		DEBUG_PRINT("Updated Configuration with version:\n%s\n", s.c_str());
		f.write((const uint8_t *)jsonText.c_str(), jsonText.length());
		f.close();
	}
	else
	{
		DEBUG_PRINT("Saving module config FAILED: %s  COULD NOT OPEN FILE\n", fileName.c_str());
	}
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
			for (byte x = 0; x < moduleCount; x++)
			{
				Module *module = modules[x];
				if (module)
				{
					for (byte index = 0; index < MAX_DEVICES; index++)
					{
						Device *device = module->getDevice(index);
						if (device)
							device->controllerLockout(lockout == LOW);
					}
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

void sendStatusMessage(void)
{
	DEBUG_PRINT("sendStatusMessage\n");
	nextModuleToNotify = 0;
}

void notificationListChanged(const JsonArray &jsonArray)
{
	jsonBuffer.clear();
	JsonObject &json = jsonBuffer.parseObject(getConfiguration());

	DEBUG_PRINT("notificationListChanged  New Count: %d\n", jsonArray.size());
	json["controllersToNotify"] = jsonArray;
	json.printTo(jsonTextToSave);
}

void setStatusIndicator(void)
{
//	DEBUG_PRINT("setStatusIndicator: %d\n", NetManager.getWiFiConnected());
	if (NetManager.getWiFiConnected())
		digitalWrite(STATUS_LED_PIN, LOW); // Turn LED on
	else
		digitalWrite(STATUS_LED_PIN, HIGH); // Turn LED off
}
