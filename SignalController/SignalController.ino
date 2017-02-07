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
#include "SignalHandler.h"
#include "ConfigStructures.h"
#include "DeviceState.h"
#include "GlobalDefs.h"

// Signal pin assignments
const int signal1RedPin = 4;
const int signal1YellowPin = 5;
const int signal1GreenPin = 2;
const int signal2RedPin = 12;
const int signal2YellowPin = 14;
const int signal2GreenPin = 16;

// Configuration EEPROM memory addresses
const int SIGNAL_CONFIG_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);

WiFiClient Tcp;

Controller controller(LocalServerPort);
SignalHandler signal1;
SignalHandler signal2;
SignalControllerConfigStruct controllerConfig;
const long heartbeatTimeout = 10 * 1000;
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
	Serial.printf("Signal Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");

  memset(&controllerConfig, 0, sizeof(SignalControllerConfigStruct));
  EEPROM.begin(4096);

  ConfigDownload.init(&controller);

  loadConfiguration();
  controller.setup(messageCallback, ClassSemaphore);

  signal1.setup(signal1RedPin, signal1YellowPin, signal1GreenPin);
  signal2.setup(signal2RedPin, signal2YellowPin, signal2GreenPin);

  DEBUG_PRINT("setup complete\n");
}

void loop() 
{
	controller.process();

	if (controller.getWiFiReconnected())
		sendHeartbeat(true);

	ConfigDownload.process();
	if (ConfigDownload.downloadComplete())
	{
		ConfigDownload.reset();
		DEBUG_PRINT("CONFIG DOWNLOAD COMPLETE!!  Saving to memory\n");
		EEPROM.put(SIGNAL_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
		loadConfiguration();
	}

	bool sendMessage = signal1.process();
	bool sendMessage2 = signal2.process();
	if (sendMessage || sendMessage2)
		sendHeartbeat(true);
	else
		sendHeartbeat(false);
} 

void loadConfiguration(void)
{
	if(controller.checkEEPROM(0xAC))
	{  
		EEPROM.get(SIGNAL_CONFIG_ADDRESS, controllerConfig);

		DEBUG_PRINT("loadConfiguration:  SignalID's: %d, %d\n", controllerConfig.signal1.signalID, controllerConfig.signal2.signalID);
		signal1.setConfig(controllerConfig.signal1);
		signal2.setConfig(controllerConfig.signal2);
	}
	else
	{
		EEPROM.put(SIGNAL_CONFIG_ADDRESS, controllerConfig);
		EEPROM.commit();
	}
}

void downloadConfig(void)
{
	// Key should be the Chip ID, single letter indicating the type of controller:
	// T = Turnout controller
	// P = Panel controller
	// S = Signal-Block controller
	// 
	// and the moduleIndex.  For this type of controller there is only one module: 0
	// The server will use this information to lookup the configuration information for this controller
	String key;
	key += controller.getControllerID();
	key += ",S,0";

	ConfigDownload.downloadConfig((uint8_t *)&controllerConfig, sizeof(SignalControllerConfigStruct), key);
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

	if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadConfig();
	}
	else if (signal1.getDeviceID() < 1 && message.getMessageID() == SYS_HEARTBEAT)
	{
		downloadConfig();
	}
	else
	{
		bool sendHeartbeat1 = signal1.handleMessage(message);
		bool sendHeartbeat2 = signal2.handleMessage(message);

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

		message.setMessageID(SIG_STATUS);
		message.setControllerID(controller.getControllerID());
		message.setMessageClass(ClassSignal);

		controller.sendNetworkMessage(message, true);
	}
}
