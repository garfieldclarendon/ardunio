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

  memset(&controllerConfig, 0, sizeof(SignalControllerConfigStruct));
  EEPROM.begin(4096);

  bool result = SPIFFS.begin();
  DEBUG_PRINT("SPIFFS opened: %d\n", result);

  ConfigDownload.init(&controller);

  controller.setup(messageCallback, ClassSemaphore);
  loadConfiguration();

  signal1.setup(motor1_pinA, motor1_pinB, normal1_pin, diverge1_pin);
  signal2.setup(motor2_pinA, motor2_pinB, normal2_pin, diverge2_pin);

  sendStatusMessage(false);

  DEBUG_PRINT("setup complete\n");
}

void loop() 
{
	controller.process();

	if (controller.getWiFiReconnected())
		sendStatusMessage(false);

	sendHeartbeatMessage();

	ConfigDownload.process();

	bool sendMessage = signal1.process();
	bool sendMessage2 = signal2.process();
	if (sendMessage || sendMessage2)
		sendStatusMessage(false);
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(SEMAPHORE_CONFIG_ADDRESS, controllerConfig);

		DEBUG_PRINT("loadConfiguration:  SignalID's: %d, %d\n", controllerConfig.signal1.signalID, controllerConfig.signal2.signalID);
		signal1.setConfig(controllerConfig.signal1);
		signal2.setConfig(controllerConfig.signal2);
		DEBUG_PRINT("signal configuration load complete\n");
	}
	else
	{
		EEPROM.put(SEMAPHORE_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();

		signal1.setConfig(controllerConfig.signal1);
		signal2.setConfig(controllerConfig.signal2);
	}
}

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
	memset(&controllerConfig, 0, sizeof(SignalControllerConfigStruct));

	ConfigDownload.reset();
	String key;
	key += ESP.getChipId();
	key += ",S,0";

	ConfigDownload.downloadConfig(key, configCallback);
}

byte currentSignalConfig = -1;
byte currentAspectIndex = 0;
byte currentConditionIndex = 0;
void configCallback(const char *key, const char *value)
{
	if (key == NULL)
	{
		ConfigDownload.reset(); 
		DEBUG_PRINT("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		EEPROM.put(SEMAPHORE_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		ESP.restart();
	}
	else
	{
		if (strcmp(key, "ID") == 0)
		{
			currentAspectIndex = -1;
			currentSignalConfig++;
			if (currentSignalConfig == 0)
				controllerConfig.signal1.signalID = atoi(value);
			else
				controllerConfig.signal2.signalID = atoi(value);
		}
		else if (strcmp(key, "ASPECT") == 0)
		{
			currentAspectIndex++;
			currentConditionIndex = -1;
		}
		else if (strcmp(key, "RED") == 0)
		{
			if (currentSignalConfig == 0)
				controllerConfig.signal1.conditions[currentAspectIndex].aspect.redMode = atoi(value);
			else
				controllerConfig.signal2.conditions[currentAspectIndex].aspect.redMode = atoi(value);
		}
		else if (strcmp(key, "YELLOW") == 0)
		{
			if (currentSignalConfig == 0)
				controllerConfig.signal1.conditions[currentAspectIndex].aspect.yellowMode = atoi(value);
			else
				controllerConfig.signal2.conditions[currentAspectIndex].aspect.yellowMode = atoi(value);
		}
		else if (strcmp(key, "GREEN") == 0)
		{
			if (currentSignalConfig == 0)
				controllerConfig.signal1.conditions[currentAspectIndex].aspect.greenMode = atoi(value);
			else
				controllerConfig.signal2.conditions[currentAspectIndex].aspect.greenMode = atoi(value);
		}
		else if (strcmp(key, "CONDITIONS") == 0)
		{
			currentConditionIndex++;
		}
		else if (strcmp(key, "DEVICEID") == 0)
		{
			if (currentSignalConfig == 0)
				controllerConfig.signal1.conditions[currentAspectIndex].conditions[currentConditionIndex].deviceID = atoi(value);
			else
				controllerConfig.signal2.conditions[currentAspectIndex].conditions[currentConditionIndex].deviceID = atoi(value);
		}
		else if (strcmp(key, "OPERAND") == 0)
		{
			if (currentSignalConfig == 0)
				controllerConfig.signal1.conditions[currentAspectIndex].conditions[currentConditionIndex].operand = atoi(value);
			else
				controllerConfig.signal2.conditions[currentAspectIndex].conditions[currentConditionIndex].operand = atoi(value);
		}
		else if (strcmp(key, "STATE") == 0)
		{
			if (currentSignalConfig == 0)
				controllerConfig.signal1.conditions[currentAspectIndex].conditions[currentConditionIndex++].deviceState = atoi(value);
			else
				controllerConfig.signal2.conditions[currentAspectIndex].conditions[currentConditionIndex++].deviceState = atoi(value);
		}
	}
}

void messageCallback(const Message &message)
{
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLOCK_STATUS)
	{
		for (byte x = 0; x < MAX_MODULES; x++)
		{
			if (message.getDeviceStatusID(x) == 0)
				break;
			DeviceState::setDeviceState(message.getDeviceStatusID(x), message.getDeviceStatus(x));
		}
	}
	else if (message.getMessageID() == SYS_SET_CONTROLLER_ID && message.getLValue() == ESP.getChipId())
	{
		downloadConfig();
	}
	else if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadConfig();
	}
	else
	{
		bool sendStatus1 = signal1.handleMessage(message);
		bool sendStatus2 = signal2.handleMessage(message);

		if (sendStatus1 || sendStatus2)
			sendStatusMessage(false);
	}
}

void sendStatusMessage(bool sendOnce)
{
	if (controller.getWiFiReconnected() > 0)
	{
		DEBUG_PRINT("Sending status message.\n");

		Message message;
		message.setMessageID(SIG_STATUS);
		message.setControllerID(controller.getControllerID());
		message.setMessageClass(ClassSemaphore);

		controller.sendNetworkMessage(message, sendOnce);
	}
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
