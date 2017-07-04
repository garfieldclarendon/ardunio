#include <ArduinoJson.h>
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
#include <FS.h>
#include <Wire.h>

#include "ConfigDownload.h"
#include "Controller.h"
#include "ConfigStructures.h"
#include "BlockModule.h"
#include "Network.h"

// Block pin assignments
const byte block1Pin = 4; // 4;
const byte block2Pin = 0;// 13;
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

  Network.setModuleCallback(netModuleCallback);
  Network.setUdpMessageCallback(udpMessageCallback);
  Network.setServerConnectedCallback(serverReconnected);

  controller.setup(ClassBlock);

  module.setup(0, logicalBlock1Pin);
  module.setup(1, logicalBlock2Pin);

  DEBUG_PRINT("setup complete\n");
}

void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	if (moduleIndex == 0)
	{
		byte data(module.getCurrentState());
		readPins(data);
		module.netModuleCallback(action, moduleIndex, json, data);
	}
}

void loop() 
{
	Network.process();
	controller.process();

	byte data(module.getCurrentState());
	readPins(data);
//	DEBUG_PRINT("after readPins: %d\n", data);

	module.process(data);
} 
byte lastRead = 0;
void readPins(byte &data)
{
	if (digitalRead(block1Pin) != lastRead)
	{
		lastRead = digitalRead(block1Pin);
		DEBUG_PRINT("readPins: %d\n", lastRead);
	}
	bitWrite(data, logicalBlock1Pin, digitalRead(block1Pin));
	bitWrite(data, logicalBlock2Pin, digitalRead(block2Pin));
}

void serverReconnected(void)
{
	DEBUG_PRINT("serverReconnected\n");
	module.sendStatusMessage();
}

void udpMessageCallback(const Message &message)
{
	controller.processMessage(message);
}
