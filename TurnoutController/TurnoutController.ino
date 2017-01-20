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
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");

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

  sendStatusMessage();

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
		EEPROM.put(TURNOUT_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		loadConfiguration();
	}

	bool sendMessage = turnout1.process();
	bool sendMessage2 = turnout2.process();
	if (sendMessage || sendMessage2)
		sendStatusMessage();
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(TURNOUT_CONFIG_ADDRESS, controllerConfig);

		turnout1.setConfig(controllerConfig.turnout1);
		turnout2.setConfig(controllerConfig.turnout2);

		// Add service to MDNS-SD
		// These are the services we want to hear FROM
		MDNS.addService("route", "tcp", LocalServerPort);
		String device("device");
		if (controllerConfig.turnout1.turnoutID > 0)
			MDNS.addService(device + controllerConfig.turnout1.turnoutID, "tcp", LocalServerPort);
		if (controllerConfig.turnout2.turnoutID > 0)
			MDNS.addService(device + controllerConfig.turnout2.turnoutID, "tcp", LocalServerPort);
	}
	else
	{
		EEPROM.put(TURNOUT_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
	}
}

void downloadConfig(void)
{
	// Key should be the ControllerID and a single letter indicating the type of controller:
	// T = Turnout controller
	// P = Panel controller
	// S = Signal-Block controller
	// 
	// The server will use this information to lookup the configuration information for this controller
	String key;
	key += controller.getControllerID();
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

		static bool sysHeatbeatHandled = false;
		if (sysHeatbeatHandled == false && message.getMessageID() == SYS_HEARTBEAT)
		{
			sendStatusMessage();
			sysHeatbeatHandled = true;
		}
		if (sendStatus1 || sendStatus2 || message.getMessageID() == PANEL_STATUS)
			sendStatusMessage();
	}
}

void sendStatusMessage(void)
{
	Message message;
	Serial.println("----------------------");
	Serial.println("Sending status message");

	message = turnout1.createMessage(turnout1.getCurrentState());
	message.setControllerID(controller.getControllerID());
	message.setIntValue1(turnout1.getTurnoutID());
	message.setIntValue2(turnout2.getTurnoutID());
	message.setByteValue1(turnout1.getCurrentState());
	message.setByteValue2(turnout2.getCurrentState());

	controller.sendNetworkMessage(message, "turnout");
	Serial.println("----------------------");
}
