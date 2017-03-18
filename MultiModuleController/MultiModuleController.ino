//#define MDNS_DEBUG_ERR
//#define MDNS_DEBUG_TX
//#define MDNS_DEBUG_RX

#define DEBUG_ESP_WIFI

#include <dummy.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FS.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
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

// MCP23008 registers (everything except direction defaults to 0)
#define IODIR   0x00   // IO direction  (0 = output, 1 = input (Default))
#define IOPOL   0x01   // IO polarity   (0 = normal, 1 = inverse)
#define GPINTEN 0x02   // Interrupt on change (0 = disable, 1 = enable)
#define DEFVAL  0x03   // Default comparison for interrupt on change (interrupts on opposite)
#define INTCON  0x04   // Interrupt control (0 = interrupt on change from previous, 1 = interrupt on change from DEFVAL)
#define IOCON   0x05   // IO Configuration: bank/mirror/seqop/disslw/haen/odr/intpol/notimp
#define GPPU    0x06   // Pull-up resistor (0 = disabled, 1 = enabled)
#define INFTF   0x07   // Interrupt flag (read only) : (0 = no interrupt, 1 = pin caused interrupt)
#define INTCAP  0x08   // Interrupt capture (read only) : value of GPIO at time of last interrupt
#define GPIO    0x09   // Port value. Write to change, read to obtain value
#define OLLAT   0x0A   // Output latch. Write to latch output.

#define BASE_ADDRESS 0x20  // MCP23008 is on I2C port 0x20

const int MODULE_CONFIG_BASE_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

byte totalModules = 1;
byte currentModuleDownload = 0;
long heartbeatTimeout = 0;

Module *modules[MAX_MODULES];
Controller controller(LocalServerPort);
MultiModuleControllerConfigStruct *controllerConfig = NULL;

enum DownloadMode
{
	DownloadNone,
	DownloadController,
	DownloadModules
} downloadMode;

void messageCallback(const Message &message) 
{
	DEBUG_PRINT("messageCallback: %d\n", message.getMessageID());
	Message returnMessage;
	if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadMode = DownloadController;
		downloadConfig();
	}
	else if (message.getMessageID() == SYS_REQEST_STATUS)
	{
		sendStatusMessage(false);
	}
	else if (message.getMessageID() == SYS_SET_CONTROLLER_ID && message.getLValue() == ESP.getChipId())
	{
		downloadMode = DownloadController;
		downloadConfig();
	}
	else if (message.getMessageID() == SYS_RESET_CONFIG && message.getLValue() == ESP.getChipId())
	{
		for (byte x = 0; x < totalModules; x++)
		{
			delete modules[x];
		}
		totalModules = 0;
		memset(modules, 0, sizeof(Module *) * MAX_MODULES);
		memset(&controllerConfig, 0, sizeof(MultiModuleControllerConfigStruct));
		loadConfiguration();
	}
	else
	{
		bool sendStatus = false;
		for (byte x = 0; x < totalModules; x++)
		{
			byte data = expanderRead(x, GPIO);
			DEBUG_PRINT("HANDLEMESSAGE  DATA: %d\n", data);
			byte hold = data;
			if (modules[x]->handleMessage(message, data))
				sendStatus = true;

			if (hold != data)
				expanderWrite(x, GPIO, data);
		}
		if (sendStatus)
			sendStatusMessage(false);
	}
	DEBUG_PRINT("messageCallback: %d DONE!!!!!\n", message.getMessageID());
}

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

	memset(modules, 0, sizeof(Module *) * MAX_MODULES);

	EEPROM.begin(512);
	bool result = SPIFFS.begin();
	DEBUG_PRINT("SPIFFS opened: %d\n", result);

	ConfigDownload.init(&controller);

	loadConfiguration();

	controller.setup(messageCallback, ClassMulti);

	setupHardware();
	for (byte x = 0; x < totalModules; x++)
	{
		byte iodir;
		iodir = modules[x]->setupWire(x);
		expanderWrite(x, IODIR, iodir);
		delay(100);
		byte data = modules[x]->getCurrentState();
		expanderWrite(x, GPIO, data);
	}

	DEBUG_PRINT("setup complete\n");
}

void setupHardware(void)
{
	DEBUG_PRINT("setup hardware\n");
	Wire.begin(); //creates a Wire object
}

void loop() 
{
	controller.process();

	if (controller.getWiFiReconnected())
	{
		sendStatusMessage(true);
	}

	sendHeartbeatMessage();

	ConfigDownload.process();

	bool sendStatus = false;
	for (byte x = 0; x < totalModules; x++)
	{
		byte data = expanderRead(x, GPIO);
		byte hold = data;
		if (modules[x]->process(data))
		{
			sendStatus = true;
		}
		if (hold != data)
			expanderWrite(x, GPIO, data);
	}
	if (sendStatus)
		sendStatusMessage(false);
}

void loadConfiguration(void)
{
	if (controller.checkEEPROM(0xAD))
	{
		// Get the number of modules connected to this controller.
		DEBUG_PRINT("Getting the controller configuration: \n");
		if (controllerConfig != NULL)
			delete controllerConfig;
		controllerConfig = new MultiModuleControllerConfigStruct;
		memset(controllerConfig, 0, sizeof(MultiModuleControllerConfigStruct));
		EEPROM.get(MODULE_CONFIG_BASE_ADDRESS, *controllerConfig);
		DEBUG_PRINT("Controller coinfiguration complete.  Total modules: %d\n", controllerConfig->mdouleCount);

		totalModules = controllerConfig->mdouleCount;

		DEBUG_PRINT("DONE GETTING CONFIGURATION\n");

		createModules();
		delete controllerConfig;
		controllerConfig = NULL;
	}
	else
	{
		DEBUG_PRINT("EEPROM not set.  Initializing to 0\n");
		// This controller has not been configured yet or the configuration is no longer valid, 
		// write default values to memory

		totalModules = 0;
		if (controllerConfig == NULL)
			controllerConfig = new MultiModuleControllerConfigStruct;
		memset(controllerConfig, 0, sizeof(MultiModuleControllerConfigStruct));
		EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, *controllerConfig);
		EEPROM.commit();
		delete controllerConfig;
		controllerConfig = NULL;
		DEBUG_PRINT("DONE SAVING EEPROM.  Initializing to 0\n");
	}
}

void createModules(void)
{
	DEBUG_PRINT("CreateModules\n");

	for (byte index = 0; index < totalModules; index++)
	{
		if (modules[index] != NULL)
			delete modules[index];

		if (controllerConfig->moduleConfigs[index].moduleClass == ClassTurnout)
		{
			DEBUG_PRINT("CreateModules:  Creating Turnout Module\n");
			TurnoutModule *turnout = new TurnoutModule();
			modules[index] = turnout;
		}

		String fileName;
		fileName = "Module_";
		fileName += index;
		fileName += ".dat";

		File f = SPIFFS.open(fileName, "r");
		if (f)
		{
			DEBUG_PRINT("Reading module config file: %s\n", f.name());
			f.read((uint8_t*)modules[index]->getConfigReference(), modules[index]->getConfigSize());
		}
		else
		{
			DEBUG_PRINT("FAILED TO READ MODULE CONFIG FILE: %s\n", f.name());
		}
	}
}

void saveControllerConfig(void)
{
	DEBUG_PRINT("Saving controller configuration to EEPROM\n");
	if (controllerConfig != NULL)
	{
		DEBUG_PRINT("Saving module configuration to EEPROM!!!!!!!\nTotal Modules: %d\n", controllerConfig->mdouleCount);

		EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, *controllerConfig);
		EEPROM.commit();
	}
}

void saveModuleConfig(void)
{
	DEBUG_PRINT("Saving module configuration to EEPROM\n");
	String fileName("Module_");
	fileName += currentModuleDownload;
	fileName += ".dat";
	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		DEBUG_PRINT("Saving Module config: %s\n", fileName.c_str());

		f.write((const uint8_t *)modules[currentModuleDownload]->getConfigReference(), modules[currentModuleDownload]->getConfigSize());
		f.close();
	}
}

void downloadConfig(void)
{
	DEBUG_PRINT("downloadConfig\n");
	// Key should be the Chip ID and a single letter indicating the type of controller:
	// T = Turnout controller
	// P = Panel controller
	// S = Signal
	// B = Block controller
	// M = Multi-Module Controller
	// 
	// The server will use this information to lookup the configuration information for this controller

	ConfigDownload.reset();
	if (downloadMode != DownloadNone)
	{
		String key;
		key += ESP.getChipId(); 

		if (downloadMode == DownloadController)
		{
			key += ",M,";
			key += downloadMode;
			controllerConfig = new MultiModuleControllerConfigStruct;
			memset(controllerConfig, 0, sizeof(MultiModuleControllerConfigStruct));
			DEBUG_PRINT("DownloadConfig.  Key: %s\n", key.c_str());
			ConfigDownload.downloadConfig(key, configCallback);
		}
		else if (downloadMode == DownloadModules)
		{
			key += ",MM,";
			key += currentModuleDownload;
			DEBUG_PRINT("DownloadConfig.  Key: %s\n", key.c_str());
			ConfigDownload.downloadConfig(key, configCallback);
		}
	}
}

int configIndex = 0;
void configCallback(const char *key, const char *value)
{
	if (key)
	{
		DEBUG_PRINT("configCallback.  Key: %s Value: %s\n", key, value);
	}
	else
	{
		DEBUG_PRINT("configCallback.  Key: IS NULL\n");
	}
	if (key == NULL)
	{
		if (downloadMode == DownloadController)
		{
			saveControllerConfig();
			if (totalModules == 0)
			{
				totalModules = controllerConfig->mdouleCount;
				createModules();
			}
			delete controllerConfig;
			controllerConfig = NULL;
			downloadMode = DownloadModules;
			downloadConfig();
		}
		else
		{
			saveModuleConfig();
			currentModuleDownload++;
			if (currentModuleDownload == totalModules)
			{
				downloadMode = DownloadNone;
				DEBUG_PRINT("configCallback.  DOWNLOAD COMPLETE  RESTARTING\n");
				controller.restart();
			}
			else
			{
				downloadConfig();
			}
		}
	}
	else
	{
		if (downloadMode == DownloadController)
		{
			if (strcmp(key, "COUNT") == 0)
			{
				int v = atoi(value);
				controllerConfig->mdouleCount = v;
			}
			else if (strcmp(key, "CLASS") == 0)
			{
				int v = atoi(value);
				controllerConfig->moduleConfigs[configIndex++].moduleClass = v;
			}

		}
		else
		{
			modules[currentModuleDownload]->configCallback(key, value);
		}
	}
}

void sendStatusMessage(bool sendOnce)
{
	DEBUG_PRINT("Sending status message\n");

	Message message;
	message.setMessageID(MULTI_STATUS);
	message.setControllerID(controller.getControllerID());
	message.setMessageClass(ClassMulti);

	byte index = 0;
	for (byte x = 0; x < totalModules; x++)
	{
		if (modules[x]->getSendModuleState())
		{
			for (byte deviceIndex = 0; deviceIndex < modules[x]->getDeviceCount(); deviceIndex++)
			{
				message.setDeviceStatus(index++, modules[x]->getDeviceID(deviceIndex), modules[x]->getDeviceState(deviceIndex));
				if (index == MAX_MODULES)
				{
					controller.sendNetworkMessage(message, true);
					index = 0;
					message.resetDeviceStatus();
				}
			}
		}
	}

	controller.sendNetworkMessage(message, true);

	// If we want to send the status message more than once, reset the heartbeat timeout to force the status
	// message on the next sendHearbeatMessage() call.
	if (sendOnce == false)
		heartbeatTimeout = 0;
}

void sendHeartbeatMessage(void)
{
	if (controller.getControllerID() > 0)
	{
		long t = millis();
		if ((t - heartbeatTimeout) > HEARTBEAT_INTERVAL)
		{
			heartbeatTimeout = t;
			sendStatusMessage(true);
		}
	}
}

void expanderWrite(byte moduleAddress, const byte reg, const byte data)
{
//	DEBUG_PRINT("expanderWrite: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, data);
	Wire.beginTransmission((byte)(BASE_ADDRESS | moduleAddress));
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission();
}

byte expanderRead(byte moduleAddress, const byte reg)
{
	Wire.beginTransmission((byte)(BASE_ADDRESS | moduleAddress));
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom((byte)(BASE_ADDRESS | moduleAddress), (byte)1);
	byte ret = Wire.read();
//	DEBUG_PRINT("expanderRead: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, ret);

	return ret;
}
