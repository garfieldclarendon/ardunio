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
#include "TurnoutHandler.h"
#include "ConfigStructures.h"

// Turnout pin assignments
const int motor1_pinA = 13;
const int motor1_pinB = 12;
const int motor2_pinA = 2;
const int motor2_pinB = 0;
const int normal2_pin = 4;
const int diverge2_pin = 5;
const int normal1_pin = 16;
const int diverge1_pin = 14;

// Configuration EEPROM memory addresses
const int TURNOUT_CONFIG_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

Controller controller(LocalServerPort);
TurnoutHandler turnout1;
TurnoutHandler turnout2;
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

  memset(&controllerConfig, 0, sizeof(TurnoutControllerConfigStruct));
  EEPROM.begin(512);

  ConfigDownload.init(&controller);

  loadConfiguration();
  controller.setup(messageCallback, ClassTurnout);

  // If the first turnout has not been assigned yet, download the configuration from the server
  if (turnout1.getTurnoutID() < 1)
	  downloadConfig();

  turnout1.setup(motor1_pinA, motor1_pinB, normal1_pin, diverge1_pin);
  turnout2.setup(motor2_pinA, motor2_pinB, normal2_pin, diverge2_pin);

  turnout1.setTurnout(TrnNormal);
  turnout2.setTurnout(TrnNormal);

  sendStatusMessage(false);

  DEBUG_PRINT("setup complete\n");
}

void loop() 
{
	controller.process();
	sendHeartbeatMessage();

	ConfigDownload.process();
	if (ConfigDownload.downloadComplete())
	{
		ConfigDownload.reset();
		DEBUG_PRINT("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		EEPROM.put(TURNOUT_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		loadConfiguration();
	}

	bool sendMessage = turnout1.process();
	bool sendMessage2 = turnout2.process();
	if (sendMessage || sendMessage2)
		sendStatusMessage(false);
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(TURNOUT_CONFIG_ADDRESS, controllerConfig);

		turnout1.setConfig(controllerConfig.turnout1);
		turnout2.setConfig(controllerConfig.turnout2);
	}
	else
	{
		EEPROM.put(TURNOUT_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
	}
}

void downloadConfig(void)
{
	// Key should be the Chip ID and a single letter indicating the type of controller:
	// T = Turnout controller
	// P = Panel controller
	// S = Signal-Block controller
	// 
	// The server will use this information to lookup the configuration information for this controller
	String key;
	key += ESP.getChipId();
	key += ",T";

	ConfigDownload.downloadConfig((uint8_t *)&controllerConfig, sizeof(TurnoutControllerConfigStruct), key);
}

void messageCallback(const Message &message)
{
	if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadConfig();
	}
	else if (turnout1.getTurnoutID() < 1 && message.getMessageID() == SYS_HEARTBEAT)
	{
		downloadConfig();
	}
	else
	{
		bool sendStatus1 = turnout1.handleMessage(message);
		bool sendStatus2 = turnout2.handleMessage(message);

		if (sendStatus1 || sendStatus2 || message.getMessageID() == PANEL_STATUS)
			sendStatusMessage(false);
	}
}

void sendStatusMessage(bool sendOnce)
{
	Message message;
	DEBUG_PRINT("----------------------\n");
	DEBUG_PRINT("Sending status message\n");

	message = turnout1.createMessage(turnout1.getCurrentState());
	message.setControllerID(controller.getControllerID());
	message.setIntValue1(turnout1.getTurnoutID());
	message.setIntValue2(turnout2.getTurnoutID());
	message.setByteValue1(turnout1.getCurrentState());
	message.setByteValue2(turnout2.getCurrentState());

	controller.sendNetworkMessage(message, sendOnce);
	DEBUG_PRINT("----------------------\n");
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
