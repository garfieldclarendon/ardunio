#include "ConfigDownload.h"
#include <WebSocketsClient.h>
#include <Hash.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "ConfigDownload.h"
#include "Controller.h"
#include "ConfigStructures.h"
#include "BlockModule.h"

// Block pin assignments
const byte block1Pin = 13; // 4;
const byte block2Pin = 4;// 13;
const byte block1LEDPin = 12;
const byte block2LEDPin = 5;
const byte logicalBlock1Pin = 0;
const byte logicalBlock2Pin = 1;

// Configuration EEPROM memory addresses
const int BLOCK_CONFIG_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

Controller controller(LocalServerPort);
BlockModule module;
BlockControllerConfigStruct controllerConfig;
long currentHeartbeatTimeout = 0;

void setup() 
{
#ifdef PROJECT_DEBUG
	Serial.begin(115200);
#else
	Serial.begin(74880);
	Serial.println();
	Serial.println();
	Serial.println("Starting up in release mode");
	Serial.printf("Block Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");

  memset(&controllerConfig, 0, sizeof(BlockControllerConfigStruct));
  EEPROM.begin(512);

  pinMode(block1Pin, INPUT);
  pinMode(block2Pin, INPUT);
  pinMode(block1LEDPin, OUTPUT);
  pinMode(block2LEDPin, OUTPUT);

  digitalWrite(block1LEDPin, LOW);
  digitalWrite(block2LEDPin, LOW);

  ConfigDownload.init(&controller);

  loadConfiguration();
  controller.setup(messageCallback, ClassBlock);

  module.setup(0, logicalBlock1Pin);
  module.setup(1, logicalBlock2Pin);

  DEBUG_PRINT("setup complete\n");
}

void loop() 
{
	controller.process();

	if (controller.getWiFiReconnected())
		sendStatusMessage(true);

	ConfigDownload.process();

	byte data(module.getCurrentState());
	readPins(data);
//	DEBUG_PRINT("after readPins: %d\n", data);

	if (module.process(data))
		sendStatusMessage(false);

	if (module.getDeviceState(0) == BlockHandler::Occupied)
		digitalWrite(block1LEDPin, HIGH);
	else
		digitalWrite(block1LEDPin, LOW);
	if (module.getDeviceState(1) == BlockHandler::Occupied)
		digitalWrite(block2LEDPin, HIGH);
	else
		digitalWrite(block2LEDPin, LOW);

	sendHeartbeat();
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(BLOCK_CONFIG_ADDRESS, controllerConfig);

		DEBUG_PRINT("loadConfiguration:  BlockID's: %d, %d\n", controllerConfig.block1.blockID, controllerConfig.block2.blockID);
		module.setConfig(0, controllerConfig.block1);
		module.setConfig(1, controllerConfig.block2);
	}
	else
	{
		EEPROM.put(BLOCK_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();

		module.setConfig(0, controllerConfig.block1);
		module.setConfig(1, controllerConfig.block2);
	}
}

void downloadConfig(void)
{
	if (controller.getControllerID() < 1)
		controller.createControllerID();

	// Key should be the Chip ID and a single letter indicating the type of controller:
	// T = Turnout controller
	// P = Panel controller
	// S = Signal
	// B = Block controller
	// M = Multi-Module Controller
	// 
	// and the moduleIndex.  For this type of controller there is only one module: 0
	// The server will use this information to lookup the configuration information for this controller
	String key;
	key += ESP.getChipId();
	key += ",B,0";

	ConfigDownload.downloadConfig(key, configCallback);
}

void messageCallback(const Message &message)
{
	if (message.getMessageID() == SYS_REQEST_STATUS && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		sendStatusMessage(true);
	}
	else if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadConfig();
	}
	else if (message.getMessageID() == SYS_RESET_CONFIG && message.getLValue() == ESP.getChipId())
	{
		memset(&controllerConfig, 0, sizeof(BlockControllerConfigStruct));
		loadConfiguration();
	}
	else
	{
		byte data(module.getCurrentState());
		readPins(data);
		if (module.handleMessage(message, data))
			sendStatusMessage(false);
	}
}

void sendHeartbeat(void)
{
	long t = millis();
	if ((t - currentHeartbeatTimeout > HEARTBEAT_INTERVAL))
	{
		currentHeartbeatTimeout = t;
		sendStatusMessage(true);
	}
}

void sendStatusMessage(bool sendOnce)
{
	DEBUG_PRINT("sendStatusMessage.  Block1: %d  Block2:  %d\n", module.getDeviceState(0), module.getDeviceState(1));
	Message message;

	message.setMessageID(BLOCK_STATUS);
	message.setControllerID(controller.getControllerID());
	message.setMessageClass(ClassBlock);
	message.setDeviceStatus(0, controllerConfig.block1.blockID, module.getDeviceState(0));
	message.setDeviceStatus(1, controllerConfig.block2.blockID, module.getDeviceState(1));

	controller.sendNetworkMessage(message, sendOnce);
}

void configCallback(const char *key, const char *value)
{
	if (key == NULL)
	{
		memset(&controllerConfig, 0, sizeof(BlockControllerConfigStruct));
		controllerConfig.block1 = module.getConfig(0);
		controllerConfig.block2 = module.getConfig(1);

		ConfigDownload.reset();
		DEBUG_PRINT("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		EEPROM.put(BLOCK_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		ESP.restart();
	}
	else
	{
		module.configCallback(key, value);
	}
}

void readPins(byte &data)
{
	bitWrite(data, logicalBlock1Pin, digitalRead(block1Pin));
	bitWrite(data, logicalBlock2Pin, digitalRead(block2Pin));
}
