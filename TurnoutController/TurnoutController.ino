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

#include "ConfigDownload.h"
#include "Controller.h"
#include "TurnoutModule.h"
#include "ConfigStructures.h"

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

	setupPins();

	memset(&controllerConfig, 0, sizeof(TurnoutControllerConfigStruct));
	EEPROM.begin(512);

	ConfigDownload.init(&controller);

	controller.setup(messageCallback, ClassTurnout);
	loadConfiguration();

	turnoutModule.setupWire(0);

	setPins();

	DEBUG_PRINT("setup complete\n");
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
	controller.process();
	if (controller.getWiFiReconnected())
	{
		sendStatusMessage(false);
	}
	sendHeartbeatMessage();

	ConfigDownload.process();

	processTurnouts();
}

void processTurnouts(void)
{
	byte data(turnoutModule.getCurrentState());
	readPins(data);
	
	bool sendMessage = turnoutModule.process(data);
//	setPins();
	if (sendMessage)
	{
		setPins();
		sendStatusMessage(false);
	}
}

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(TURNOUT_CONFIG_ADDRESS, controllerConfig);

		DEBUG_PRINT("LOAD TURNOUT1: %d TURNOUT2: %d \n", controllerConfig.turnout1.turnoutID, controllerConfig.turnout2.turnoutID);
		turnoutModule.setConfig(0, controllerConfig.turnout1);
		turnoutModule.setConfig(1, controllerConfig.turnout2);
	}
	else
	{
		EEPROM.put(TURNOUT_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();

		turnoutModule.setConfig(0, controllerConfig.turnout1);
		turnoutModule.setConfig(1, controllerConfig.turnout2);
	}
}

int currentTurnoutConfig = -1;
int currentRouteConfig = 0;
void downloadConfig(void)
{
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
	key += ",T,0";

	ConfigDownload.downloadConfig(key, configCallback);
}

void configCallback(const char *key, const char *value)
{
	if (key == NULL)
	{
		ConfigDownload.reset();
		DEBUG_PRINT("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		memcpy(&controllerConfig, turnoutModule.getConfigReference(), turnoutModule.getConfigSize());
		DEBUG_PRINT("SAVING TURNOUT1: %d TURNOUT2: %d \n",controllerConfig.turnout1.turnoutID, controllerConfig.turnout2.turnoutID);
		EEPROM.put(TURNOUT_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		controller.restart();
	}
	else
	{
		turnoutModule.configCallback(key, value);
	}
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
	else if (message.getMessageID() == SYS_SET_CONTROLLER_ID && message.getLValue() == ESP.getChipId())
	{
		downloadConfig();
	}
	else
	{
		byte data(turnoutModule.getCurrentState());
		readPins(data);
		bool sendStatus = turnoutModule.handleMessage(message, data);
//		setPins();

		if (sendStatus)
		{
			setPins();
			sendStatusMessage(false);
		}
	}
}

void sendStatusMessage(bool sendOnce)
{
	Message message;
	DEBUG_PRINT("----------------------\n");
	DEBUG_PRINT("Sending status message\n");

	message = turnoutModule.createMessage();
	message.setControllerID(controller.getControllerID());
	controller.sendNetworkMessage(message, sendOnce);
	DEBUG_PRINT("----------------------\n");
}

void setPins(void)
{
	byte data = turnoutModule.getCurrentState();
//	DEBUG_PRINT("DATA %d\n", data);

	digitalWrite(motor1_pinA, bitRead(data, motor1_logicalPinA));
	DEBUG_PRINT("Morotr1_A %d\n", bitRead(data, motor1_logicalPinA));
	digitalWrite(motor1_pinB, bitRead(data, motor1_logicalPinB));
	DEBUG_PRINT("Morotr1_B %d\n", bitRead(data, motor1_logicalPinB));
	digitalWrite(motor2_pinA, bitRead(data, motor2_logicalPinA));
	DEBUG_PRINT("Morotr2_A %d\n", bitRead(data, motor2_logicalPinA));
	digitalWrite(motor2_pinB, bitRead(data, motor2_logicalPinB));
	DEBUG_PRINT("Morotr2_B %d\n", bitRead(data, motor2_logicalPinB));
}

void readPins(byte &data)
{
	bitWrite(data, diverge1_logicalPin, digitalRead(diverge1_pin));
	bitWrite(data, normal1_logicalPin, digitalRead(normal1_pin));
	bitWrite(data, diverge2_logicalPin, digitalRead(diverge2_pin));
	bitWrite(data, normal2_logicalPin, digitalRead(normal2_pin));
}

long heartbeatTimeout = 0;
void sendHeartbeatMessage(void)
{
	long t = millis();
	if ((t - heartbeatTimeout) > HEARTBEAT_INTERVAL)
	{
		heartbeatTimeout = t;
		sendStatusMessage(true);
	}
}
