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
#include "BlockHandler.h"

// Block pin assignments
const int block1Pin = 4;// 13;
const int block2Pin = 13;// 4;

// Configuration EEPROM memory addresses
const int BLOCK_CONFIG_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

WiFiClient Tcp;

Controller controller(LocalServerPort);
BlockHandler block1;
BlockHandler block2;
BlockControllerConfigStruct controllerConfig;
const long heartbeatTimeout = 5 * 1000;
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

  ConfigDownload.init(&controller);

  loadConfiguration();
  controller.setup(messageCallback, ClassBlock);

  block1.setup(block1Pin);
  block2.setup(block2Pin);

  DEBUG_PRINT("setup complete\n");
}

void loop() 
{
	controller.process();

	if (controller.getWiFiReconnected())
		sendStatusMessage(true);

	ConfigDownload.process();
	if (ConfigDownload.downloadComplete())
	{
		ConfigDownload.reset();
		DEBUG_PRINT("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		EEPROM.put(BLOCK_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		loadConfiguration();
	}

	bool sendMessage = block1.process();
	bool sendMessage2 = block2.process();
	if (sendMessage || sendMessage2)
		sendStatusMessage(false);
	else
		sendStatusMessage(false);
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(BLOCK_CONFIG_ADDRESS, controllerConfig);

		DEBUG_PRINT("loadConfiguration:  BlockID's: %d, %d\n", controllerConfig.block1.blockID, controllerConfig.block2.blockID);
		block1.setConfig(controllerConfig.block1);
		block2.setConfig(controllerConfig.block2);
	}
	else
	{
		EEPROM.put(BLOCK_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
	}
}

void downloadConfig(void)
{
	// Key should be the ControllerID and a single letter indicating the type of controller:
	// T = Turnout controller
	// P = Panel controller
	// S = Signal controller
	// B = Block controller
	//
	// The server will use this information to lookup the configuration information for this controller
	String key;
	key += controller.getControllerID();
	key += ",B";

	ConfigDownload.downloadConfig((uint8_t *)&controllerConfig, sizeof(BlockControllerConfigStruct), key);
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
	else if (block1.getBlockID() < 1 && message.getMessageID() == SYS_HEARTBEAT)
	{
		downloadConfig();
	}
	else
	{
		bool sendStatus1 = block1.handleMessage(message);
		bool sendStatus2 = block2.handleMessage(message);

		if (sendStatus1 || sendStatus2)
			sendStatusMessage(false);
	}
}

void sendHeartbeat(void)
{
	long t = millis();
	if ((t - currentHeartbeatTimeout > heartbeatTimeout))
	{
		currentHeartbeatTimeout = t;
		sendStatusMessage(true);
	}
}

void sendStatusMessage(bool sendOnce)
{
	Message message;

	message.setMessageID(BLOCK_STATUS);
	message.setControllerID(controller.getControllerID());
	message.setMessageClass(ClassBlock);
	message.setDeviceStatus(0, block1.getBlockID(), block1.getCurrentState());
	message.setDeviceStatus(1, block2.getBlockID(), block2.getCurrentState());

	controller.sendNetworkMessage(message, sendOnce);
}