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
#include <ArduinoJson.h>

#include "Controller.h"
#include "SemaphoreHandler.h"
#include "ConfigStructures.h"
#include "DeviceState.h"
#include "Network.h"

// Semaphore pin assignments
const int motor1_pinA = 13;
const int motor1_pinB = 12;
const int motor2_pinA = 2;
const int motor2_pinB = 0;
const int normal2_pin = 4;
const int diverge2_pin = 5;
const int normal1_pin = 16;
const int diverge1_pin = 14;

// Configuration EEPROM memory addresses
const int SEMAPHORE_CONFIG_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

Controller controller(LocalServerPort);
SemaphoreHandler signal1;
SemaphoreHandler signal2;

void setup() 
{
#ifdef PROJECT_DEBUG
	Serial.begin(115200);
#else
	Serial.begin(74880);
	Serial.println();
	Serial.println();
	Serial.println("Starting up in release mode");
	Serial.printf("Semaphone Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");

  EEPROM.begin(4096);

  bool result = SPIFFS.begin();
  DEBUG_PRINT("SPIFFS opened: %d\n", result);

  controller.setup(ClassSemaphore);

  Network.setModuleCallback(netModuleCallback);
  Network.setUdpMessageCallback(udpMessageCallback);
  Network.setServerConnectedCallback(serverReconnected);

  signal1.setup(motor1_pinA, motor1_pinB, normal1_pin, diverge1_pin);
  signal2.setup(motor2_pinA, motor2_pinB, normal2_pin, diverge2_pin);

  DEBUG_PRINT("setup complete\n");
}

void loop() 
{
	Network.process();
	controller.process();

	if (Network.getIsConnected())
	{
		signal1.process();
		signal2.process();
	}
	else
	{
		// We're disconnected from the server, set the signal to Red/Stop
		digitalWrite(motor1_pinA, 1);
		digitalWrite(motor1_pinB, 0);

		digitalWrite(motor2_pinA, 1);
		digitalWrite(motor2_pinB, 0);
	}
} 

void netModuleCallback(NetActionType action, byte, const JsonObject &root)
{
	if (action == NetActionUpdate)
	{
		int port = root["port"];
		byte motorPinSetting = root["motorPinSetting"];

		if (port == 0)
			signal1.setSignal(motorPinSetting);
		else
			signal2.setSignal(motorPinSetting);
	}
}

void udpMessageCallback(const Message &message)
{
	controller.processMessage(message);
}

void serverReconnected(void)
{
	DEBUG_PRINT("serverReconnected\n");
	StaticJsonBuffer<500> jsonBuffer;
	JsonObject &out = jsonBuffer.createObject();
	out["messageUri"] = "/controller/module";
	out["moduleIndex"] = 0;
	out["class"] = (int)ClassSemaphore;
	out["action"] = (int)NetActionGet;
	Network.sendMessageToServer(out);
}
