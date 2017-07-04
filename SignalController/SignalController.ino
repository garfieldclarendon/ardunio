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

#include "Controller.h"
#include "SignalModule.h"
#include "ConfigStructures.h"
#include "Network.h"

// Signal pin assignments
const int signal1Pin1 = 4;
const int signal1Pin2 = 5;
const int signal1Pin3 = 2;
const int signal2Pin1 = 12;
const int signal2Pin2 = 14;
const int signal2Pin3 = 16;

// Configuration EEPROM memory addresses
const int SIGNAL_CONFIG_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

WiFiClient Tcp;

Controller controller(LocalServerPort);
SignalModule signalModule;

void setup() 
{
#ifdef PROJECT_DEBUG
	Serial.begin(115200);
#else
	Serial.begin(74880);
	Serial.println();
	Serial.println();
	Serial.println("Starting up in release mode");
	Serial.printf("Signal Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");

  EEPROM.begin(512);
  controller.setup(ClassSignal);

  Network.setModuleCallback(netModuleCallback);
  Network.setUdpMessageCallback(udpMessageCallback);
  Network.setServerConnectedCallback(serverReconnected);

  signalModule.setup();
  byte data = 0;
  signalModule.process(data);
  setPins();

  DEBUG_PRINT("setup complete\n");
}

void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	if (moduleIndex == 0)
	{
		byte data(signalModule.getCurrentState());
		signalModule.netModuleCallback(action, moduleIndex, json, data);
		setPins();
	}
}

void loop() 
{
	Network.process();
	controller.process();
}

void setPins(void)
{
	byte data = signalModule.getCurrentState();

	digitalWrite(signal1Pin1, bitRead(data, 0));
	digitalWrite(signal1Pin2, bitRead(data, 1));
	digitalWrite(signal1Pin3, bitRead(data, 2));

	digitalWrite(signal2Pin1, bitRead(data, 3));
	digitalWrite(signal2Pin2, bitRead(data, 4));
	digitalWrite(signal2Pin3, bitRead(data, 5));
}

void serverReconnected(void)
{
	DEBUG_PRINT("serverReconnected\n");
}

void udpMessageCallback(const Message &message)
{
	controller.processMessage(message);
}
