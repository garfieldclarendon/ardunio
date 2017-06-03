#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <FS.h>
#include <Wire.h>

#include "Controller.h"
#include "TurnoutModule.h"
#include "ConfigStructures.h"
#include "Network.h"

// Turnout pin assignments
const byte motor1_pinA = 13;
const byte motor1_pinB = 12;
const byte motor2_pinA = 2;
const byte motor2_pinB = 0;
const byte normal2_pin = 4;
const byte diverge2_pin = 5;
const byte normal1_pin = 16;
const byte diverge1_pin = 14;

const byte motor1_logicalPinA = 0;
const byte motor1_logicalPinB = 1;
const byte motor2_logicalPinA = 4;
const byte motor2_logicalPinB = 5;
const byte normal1_logicalPin = 2;
const byte diverge1_logicalPin = 3;
const byte normal2_logicalPin = 6;
const byte diverge2_logicalPin = 7;

// Configuration EEPROM memory addresses
const int TURNOUT_CONFIG_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

Controller controller(LocalServerPort);

TurnoutModule turnoutModule;
TurnoutControllerConfigStruct controllerConfig;

void netModuleConfigCallback(NetActionType action, byte moduleIndex, const JsonObject &json);
void netControllerStatusCallback(int &controllerID, String &controllerName, String &controllerType, String &currentStatus);
void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json);

void setup() 
{
#ifdef PROJECT_DEBUG
	Serial.begin(115200);
#else
	Serial.begin(74880);
	Serial.println();
	Serial.println();
	Serial.println("Starting up in release mode");
	Serial.printf("Turnout Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");

	SPIFFS.begin();
	setupPins();

	memset(&controllerConfig, 0, sizeof(TurnoutControllerConfigStruct));
	EEPROM.begin(512);

	controller.setup(ClassTurnout);

	Network.setModuleConfigCallback(netModuleConfigCallback);
	Network.setControllerStatusCallback(netControllerStatusCallback);
	Network.setModuleCallback(netModuleCallback);
	Network.setUdpMessageCallback(udpMessageCallback);
	Network.setServerConnectedCallback(serverReconnected);

	loadConfiguration();

	turnoutModule.setupWire(255);
	byte data = 0;
	readPins(data);
	turnoutModule.process(data);
	setPins();

	DEBUG_PRINT("setup complete\n");
}

void netControllerStatusCallback(int &controllerID, String &controllerName, String &controllerType, String &currentStatus)
{
	controllerID = controller.getControllerID();
	controllerName = controller.getControllerName();
	controllerType = "Turnout";

	currentStatus  = "<table>";
	currentStatus += "<tr>";
	currentStatus += "<th>Device ID</th>";
	currentStatus += "<th>Status</th>";
	currentStatus += "</tr>";
	currentStatus += "<tr>";
	currentStatus += "<td>";
	currentStatus += turnoutModule.getDeviceID(0);
	currentStatus += "</td>";
	currentStatus += "<td>";
	currentStatus += turnoutModule.getCurrentState();
	currentStatus += "</td>";
	currentStatus += "</tr>";
	currentStatus += "<tr>";
	currentStatus += "<td>";
	currentStatus += turnoutModule.getDeviceID(1);
	currentStatus += "</td>";
	currentStatus += "<td>";
	currentStatus += turnoutModule.getCurrentState();
	currentStatus += "</td>";
	currentStatus += "</tr>";
	currentStatus += "</table>";
}

void netModuleConfigCallback(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	DEBUG_PRINT("netConfigCallback: NetAction %d\n", action);

	if (moduleIndex == 0)
	{
		if (action == NetActionUpdate)
			saveConfig(json);
		else if (action == NetActionDelete)
			deleteConfig();

		turnoutModule.netModuleConfigCallback(action, moduleIndex, json);
	}
}

void saveConfig(const JsonObject &json)
{
	String txt;
	json.printTo(txt);

	String fileName("/Module_0.json");
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

void deleteConfig(void)
{
	String fileName("/Module_0.json");
	SPIFFS.remove(fileName);
}

void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	if (moduleIndex == 0)
	{
		byte data(turnoutModule.getCurrentState());
		readPins(data);
		turnoutModule.netModuleCallback(action, moduleIndex, json, data);
		setPins();
	}
}

void setupPins(void)
{
	pinMode(motor1_pinA, OUTPUT);
	pinMode(motor2_pinA, OUTPUT);
	pinMode(motor1_pinB, OUTPUT);
	pinMode(motor2_pinB, OUTPUT);

	pinMode(normal1_pin, INPUT);
	pinMode(diverge1_pin, INPUT);
	pinMode(normal2_pin, INPUT);
	pinMode(diverge2_pin, INPUT);
}

void loop() 
{
	Network.process();
	controller.process();
	processTurnouts();
}

void processTurnouts(void)
{
	byte data(turnoutModule.getCurrentState());
	readPins(data);
	
	bool sendMessage = turnoutModule.process(data);
	setPins();
	if (sendMessage)
	{
		turnoutModule.sendStatusMessage();
	}
}

void loadConfiguration(void)
{
	DEBUG_PRINT("LOAD CONFIGURATION\n");
	String json;
	String fileName("/Module_0.json");
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
			turnoutModule.netModuleConfigCallback(NetActionUpdate, 0, root);
		}
	}
	else
	{
		DEBUG_PRINT("Configuration file %s is missing or can not be opened\n", fileName.c_str());
	}
}

void setPins(void)
{
	byte data = turnoutModule.getCurrentState();
//	DEBUG_PRINT("DATA %d\n", data);

	digitalWrite(motor1_pinA, bitRead(data, motor1_logicalPinA));
//	DEBUG_PRINT("Morotr1_A %d\n", bitRead(data, motor1_logicalPinA));
	digitalWrite(motor1_pinB, bitRead(data, motor1_logicalPinB));
//	DEBUG_PRINT("Morotr1_B %d\n", bitRead(data, motor1_logicalPinB));
	digitalWrite(motor2_pinA, bitRead(data, motor2_logicalPinA));
//	DEBUG_PRINT("Morotr2_A %d\n", bitRead(data, motor2_logicalPinA));
	digitalWrite(motor2_pinB, bitRead(data, motor2_logicalPinB));
//	DEBUG_PRINT("Morotr2_B %d\n", bitRead(data, motor2_logicalPinB));
}

void readPins(byte &data)
{
	bitWrite(data, diverge1_logicalPin, digitalRead(diverge1_pin));
	bitWrite(data, normal1_logicalPin, digitalRead(normal1_pin));
	bitWrite(data, diverge2_logicalPin, digitalRead(diverge2_pin));
	bitWrite(data, normal2_logicalPin, digitalRead(normal2_pin));
}

void serverReconnected(void)
{
	DEBUG_PRINT("serverReconnected\n");
	turnoutModule.sendStatusMessage();
}

void udpMessageCallback(const Message &message)
{
	controller.processMessage(message);
}
