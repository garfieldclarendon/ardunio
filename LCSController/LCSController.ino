#define DEBUG_ESP_WIFI

#include <dummy.h>
#include <Hash.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

#include "Controller.h"
#include "GlobalDefs.h"
#include "Structures.h"
#include "Device.h"
#include "NetworkManager.h"

#define BASE_ADDRESS 0x20  // MCP23008 is on I2C port 0x20

const int MODULE_CONFIG_BASE_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

// STATUS_LED_PIN is used to show the status of the connection to the WiFi network
// LED on indicates the controller is connected
// LED off indicates the controller is offline
const byte STATUS_LED_PIN = 2;// use the built in blue LED on the ESP8266 connected to pin2
const byte LOCKOUT_PIN = 13;
bool lockout = false;
byte lastLockoutRead = 0;
long currentLockoutTimeout = 0;

byte totalDevices = 0;
Device *firstDevice = NULL;
bool downloadConfigFlag = false;

Controller controller(LocalServerPort);
ControllerConfigStruct controllerConfig;

void setup() 
{
#ifdef PROJECT_DEBUG
	Serial.begin(115200);
#else
	Serial.begin(74880);
	Serial.println();
	Serial.println();
	Serial.println("Starting up in production mode");
	Serial.printf("LCS Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");
	memset(&controllerConfig, 0, sizeof(ControllerConfigStruct));

	EEPROM.begin(512);
	bool result = SPIFFS.begin();
	DEBUG_PRINT("SPIFFS opened: %d\n", result);

	loadConfiguration();

	controller.setServerFoundCallback(serverFound);
	controller.setup((ClassEnum)controllerConfig.controllerClass);

	NetManager.setUdpMessageCallback(udpMessageCallback);
	NetManager.setWIFIConnectCallback(networkConnected);

	Device *device = firstDevice;
	if (device)
	{
		setupHardware();
		createDevices();
		loadDeviceConfiguration();

		byte x = 0;
		while (device)
		{
			if (controllerConfig.controllerClass == ClassMulti)
			{
				DEBUG_PRINT("CALLING SETUPWIRE FOR ADDRESS: %d\n", controllerConfig.deviceConfigs[x].address);
				device->setupWire(controllerConfig.deviceConfigs[x].address, controllerConfig.deviceConfigs[x].port);
			}
			else
			{
				DEBUG_PRINT("CALLING SETUP\n");
				device->setup(controllerConfig.deviceConfigs[x].port);
			}
			device = device->getNextDevice();
			x++;
		}
	}

	DEBUG_PRINT("setup complete\n");
}
void serverFound(void)
{
	DEBUG_PRINT("networkConnected\n");

	if (downloadConfigFlag)
	{
		downloadConfig();
	}
}

void networkConnected(bool connected)
{
	Device *device = firstDevice;
	while (device)
	{
		if (connected)
			device->networkOnline();
		else
			device->networkOffline();
		device->sendStatusMessage();
		device = device->getNextDevice();
	}
}

void udpMessageCallback(const UDPMessage &message)
{
	controller.processMessage(message);
	Device *device = firstDevice;
	while (device)
	{
		if (device->getID() == message.getID())
		{
			device->processUDPMessage(message);
			device = NULL;
		}
		else
		{
			device = device->getNextDevice();
		}
	}
}

void setupHardware(void)
{
	DEBUG_PRINT("Setup Hardware\n");
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
	if (NetManager.process())
		controller.findServer();
	controller.process();

	Device *device = firstDevice;
	while (device)
	{
		if (controllerConfig.controllerClass == ClassMulti)
			device->processWire();
		else
			device->processNoWire();
		yield();
		device = device->getNextDevice();
	}

	if (controllerConfig.controllerClass == ClassMulti)
		setStatusIndicator();

	if (totalDevices == 0)
		delay(100);
}

void loadConfiguration(void)
{
	if (EEPROM.read(0) == 0xBB)
	{
		// Get the number of modules connected to this controller.
		DEBUG_PRINT("Getting the controller configuration: \n");
		EEPROM.get(MODULE_CONFIG_BASE_ADDRESS, controllerConfig);
		DEBUG_PRINT("Controller configuration complete.  Total devices: %d\n", controllerConfig.deviceCount);

		totalDevices = controllerConfig.deviceCount;

		DEBUG_PRINT("DONE GETTING CONFIGURATION\n");
	}
	else
	{
		DEBUG_PRINT("EEPROM not set.  Initializing to 0\n");
		// This controller has not been configured yet or the configuration is no longer valid, 
		// write default values to memory

		totalDevices = 0;
		memset(&controllerConfig, 0, sizeof(ControllerConfigStruct));
		saveControllerConfig();
		DEBUG_PRINT("DONE SAVING EEPROM.  Initializing to 0\n");
		downloadConfigFlag = true;
	}
}

void loadDeviceConfiguration(void)
{
	DEBUG_PRINT("LOAD DEVICE CONFIGURATION\n");
	Device *device = firstDevice;
	while (device)
	{
		String json;
		String fileName("/Device_");
		fileName += device->getAddress();
		fileName += "_";
		fileName += device->getPort();
		fileName += ".json";
		File f = SPIFFS.open(fileName, "r");

		if (f)
		{
			DEBUG_PRINT("Reading Device config: %s\n", fileName.c_str());

			json = f.readString();
			f.close();

			if (json.length() > 0)
			{
				StaticJsonBuffer<512> jsonBuffer;
				JsonObject &root = jsonBuffer.parseObject(json);
				device->netDeviceConfigCallback(NetActionUpdate, device->getAddress(), device->getPort(), root);
			}
		}
		else
		{
			DEBUG_PRINT("Configuration file %s is missing or can not be opened\n", fileName.c_str());
		}
		device = device->getNextDevice();
	}
}

void createDevices(void)
{
	DEBUG_PRINT("CreateDevices\n");
//	bool allDevicesCreated = true;

	while (firstDevice)
	{
		Device *hold = firstDevice;
		firstDevice = firstDevice->getNextDevice();
		delete hold;

	}
	firstDevice = NULL;
	for (byte x = 0; x > totalDevices; x++)
	{
		Device *device = NULL;
		switch (controllerConfig.deviceConfigs[x].deviceClass)
		{
			case ClassTurnout:
			{
				DEBUG_PRINT("CreateModules:  Creating Turnout Module\n");
//				device = new TurnoutModule();
				break;
			}
			case ClassBlock:
			case ClassInput:
			{
				DEBUG_PRINT("CreateModules:  Creating Input Module\n");
//				device = new InputModule();
				break;
			}
			case ClassSignal:
			case ClassOutput:
			{
				DEBUG_PRINT("CreateModules:  Creating Output Module\n");
//				device = new OutputModule();
				break;
			}
			default:
			{
				DEBUG_PRINT("CreateDevices:  INVALID DEVICE CLASS: %d\n", controllerConfig.deviceConfigs[x].deviceClass);
//				allDevicesCreated = false;
				totalDevices = 0;
				break;
			}
		}
		if (device)
		{
			if (firstDevice)
				firstDevice->setNextDevice(device);
			else
				firstDevice = device;
		}
	}
}

void downloadConfig(void)
{
	DEBUG_PRINT("downloadConfig\n");

	downloadConfigFlag = false;
	StaticJsonBuffer<200> jsonBuffer;
	String payload = NetManager.getControllerConfig(ESP.getChipId());
	JsonObject &json = jsonBuffer.parseObject(payload);

	controllerConfig.controllerClass = (ClassEnum)(int)json["controllerClass"];

	JsonArray &devices = json["devices"];
	controllerConfig.controllerClass = json["controllerClass"];
	controllerConfig.deviceCount = devices.size();
	for (byte x = 0; x < devices.size(); x++)
	{
		controllerConfig.deviceConfigs[x].deviceClass = devices[x]["class"];
		controllerConfig.deviceConfigs[x].address = devices[x]["address"];
		controllerConfig.deviceConfigs[x].port = devices[x]["port"];
		DEBUG_PRINT("Device Config  Class: %d  Address: %d Port: %d\n", controllerConfig.deviceConfigs[x].deviceClass, controllerConfig.deviceConfigs[x].address, controllerConfig.deviceConfigs[x].port);
	}

	saveControllerConfig();
	controller.restart();
}

void saveControllerConfig(void)
{
	DEBUG_PRINT("Saving module configuration to EEPROM!!!!!!!\nTotal Devices: %d\n", controllerConfig.deviceCount);
	
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
			Device *device = firstDevice;
			while (device)
			{
				device->controllerLockout(lockout == LOW);
				device = device->getNextDevice();
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
	if (NetManager.getWiFiConnected())
		digitalWrite(STATUS_LED_PIN, LOW); // Turn LED on
	else
		digitalWrite(STATUS_LED_PIN, HIGH); // Turn LED off
}
