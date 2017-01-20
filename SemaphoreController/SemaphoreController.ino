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
#include "SemaphoreHandler.h"
#include "ConfigStructures.h"
#include "DeviceState.h"

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

WiFiClient Tcp;

Controller controller(LocalServerPort);
SemaphoreHandler signal1;
SemaphoreHandler signal2;
SignalControllerConfigStruct controllerConfig;
const long heartbeatTimeout = 10 * 1000;
long currentHeartbeatTimeout = 0;

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  
  memset(&controllerConfig, 0, sizeof(SignalControllerConfigStruct));
  EEPROM.begin(4096);

  ConfigDownload.init(&controller);

  loadConfiguration();
  controller.setup(messageCallback, ClassSemaphore);

  // Add service to MDNS-SD
  // These are the services we want to hear FROM
  MDNS.addService("turnout", "tcp", LocalServerPort);
  MDNS.addService("block", "tcp", LocalServerPort);

  signal1.setup(motor1_pinA, motor1_pinB, normal1_pin, diverge1_pin);
  signal2.setup(motor2_pinA, motor2_pinB, normal2_pin, diverge2_pin);

  Serial.println("setup complete");
}

void loop() 
{
	static bool statusSent = false;
	if (statusSent == false)
	{
		statusSent = true;
		sendStatusMessage();
	}

	controller.process();
	ConfigDownload.process();
	if (ConfigDownload.downloadComplete())
	{
		ConfigDownload.reset();
		Serial.printf("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		EEPROM.put(SEMAPHORE_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		loadConfiguration();
	}

	bool sendMessage = signal1.process();
	bool sendMessage2 = signal2.process();
	if (sendMessage || sendMessage2)
		sendStatusMessage();
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(SEMAPHORE_CONFIG_ADDRESS, controllerConfig);

		Serial.printf("loadConfiguration:  SignalID's: %d, %d\n", controllerConfig.signal1.signalID, controllerConfig.signal2.signalID);
		signal1.setConfig(controllerConfig.signal1);
		signal2.setConfig(controllerConfig.signal2);
	}
	else
	{
		EEPROM.put(SEMAPHORE_CONFIG_ADDRESS, controllerConfig);
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
	key += ",S";

	ConfigDownload.downloadConfig((uint8_t *)&controllerConfig, sizeof(SignalControllerConfigStruct), key);
}

void messageCallback(const Message &message)
{
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLOCK_STATUS)
	{
		DeviceState::setDeviceState(message.getIntValue1(), message.getByteValue1());
		DeviceState::setDeviceState(message.getIntValue2(), message.getByteValue2());
	}

	if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadConfig();
	}
	else if (signal1.getSignalID() < 1 && message.getMessageID() == SYS_HEARTBEAT)
	{
		downloadConfig();
	}
	else
	{
		bool sendStatus1 = signal1.handleMessage(message);
		bool sendStatus2 = signal2.handleMessage(message);

		if (sendStatus1 || sendStatus2)
			sendStatusMessage();
	}
}

void sendStatusMessage(void)
{
	Serial.println("Sending status message.");

	Message message;
	message.setMessageID(SIG_STATUS);
	message.setControllerID(controller.getControllerID());
	message.setMessageClass(ClassSemaphore);

	controller.sendNetworkMessage(message, "signal");
}
