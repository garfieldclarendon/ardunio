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

Controller controller;
BlockHandler block1;
BlockHandler block2;
BlockControllerConfigStruct controllerConfig;
const long heartbeatTimeout = 5 * 1000;
long currentHeartbeatTimeout = 0;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  
  memset(&controllerConfig, 0, sizeof(BlockControllerConfigStruct));
  EEPROM.begin(512);

  ConfigDownload.init(&controller);

  loadConfiguration();
  controller.setup(messageCallback, ClassBlock);

  block1.setup(block1Pin);
  block2.setup(block2Pin);

  Serial.println("setup complete");
}

void loop() 
{
	controller.process();
	ConfigDownload.process();
	if (ConfigDownload.downloadComplete())
	{
		ConfigDownload.reset();
		Serial.printf("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		EEPROM.put(BLOCK_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		loadConfiguration();
	}

	bool sendMessage = block1.process();
	bool sendMessage2 = block2.process();
	if (sendMessage || sendMessage2)
		sendHeartbeat(true);
	else
		sendHeartbeat(false);
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(BLOCK_CONFIG_ADDRESS, controllerConfig);

		Serial.printf("loadConfiguration:  BlockID's: %d, %d\n", controllerConfig.block1.blockID, controllerConfig.block2.blockID);
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
	if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadConfig();
	}
	else if (block1.getBlockID() < 1 && message.getMessageID() == SYS_HEARTBEAT)
	{
		downloadConfig();
	}
	else
	{
		bool sendHeartbeat1 = block1.handleMessage(message);
		bool sendHeartbeat2 = block2.handleMessage(message);

		if (sendHeartbeat1 || sendHeartbeat2)
			sendHeartbeat(true);
	}
}

void sendHeartbeat(bool forceSend)
{
	Message message;
	long t = millis();
	if (forceSend || (t - currentHeartbeatTimeout > heartbeatTimeout))
	{
		currentHeartbeatTimeout = t;

		message.setMessageID(BLOCK_STATUS);
		message.setControllerID(controller.getControllerID());
		message.setMessageClass(ClassBlock);
		message.setIntValue1(block1.getBlockID());
		message.setByteValue1(block1.getCurrentState());
		message.setIntValue2(block2.getBlockID());
		message.setByteValue2(block2.getCurrentState());

		controller.sendNetworkMessage(message);
	}
}
