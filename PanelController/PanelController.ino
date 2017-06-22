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

#include "GlobalDefs.h"
#include "PanelModule.h"
#include "Controller.h"
#include "configStructures.h"
#include "Network.h"

bool buttonPressed = false;
bool downloadConfigFlag= false;
PanelControllerConfigStruct config;

PanelModuleClass modules[MAX_PANEL_MODULES];
Controller controller(LocalServerPort);

void setup() 
{
#ifdef PROJECT_DEBUG
	Serial.begin(115200);
#else
	Serial.begin(74880);
	Serial.println();
	Serial.println();
	Serial.println("Starting up in release mode");
	Serial.printf("Panel Controller Version: %d\n\n\n", ControllerVersion);
	Serial.flush();
	Serial.end();
#endif
	DEBUG_PRINT("setup\n");

	memset(&config, 0, sizeof(PanelControllerConfigStruct));
	config.totalModules = 1;
	memset(&modules, 0, sizeof(PanelModuleClass) * MAX_PANEL_MODULES);
	EEPROM.begin(1024);

	bool result = SPIFFS.begin();
	DEBUG_PRINT("SPIFFS opened: %d\n", result);

	controller.setup(ClassPanel);

	Network.setControllerCallback(netControllerCallback);
	Network.setControllerConfigCallback(netControllerConfigCallback);
	Network.setModuleCallback(netModuleCallback);
	Network.setUdpMessageCallback(udpMessageCallback);
	Network.setServerConnectedCallback(serverReconnected);

	loadConfig();
	setupHardware();
	for (byte x = 0; x < config.totalModules; x++)
	{
		modules[x].setup(x);
	}

	DEBUG_PRINT("setup complete\n");
}

void setupHardware(void)
{
	DEBUG_PRINT("setup hardware\n");
	Wire.begin(4,5); //creates a Wire object

	controller.addExtraPin(0, 2, config.extraPin0Mode);
	controller.addExtraPin(1, 0, config.extraPin1Mode);
	controller.addExtraPin(2, 12, config.extraPin2Mode);
	controller.addExtraPin(3, 14, config.extraPin3Mode);
	controller.addExtraPin(4, 16, config.extraPin4Mode);
}

void loop() 
{
	Network.process();
	controller.process();
	for (byte x = 0; x < config.totalModules; x++)
	{
		modules[x].setFlashAll(Network.getIsConnected() == false);
		modules[x].process(true);
	}
}

void udpMessageCallback(const Message &message)
{
	controller.processMessage(message);
}

String netControllerConfigCallback(NetActionType action, const JsonObject &root)
{
	DEBUG_PRINT("netControllerConfigCallback: NetAction %d\n", action);

	JsonArray &mods = root["modules"];
	config.totalModules = mods.size();

	config.extraPin0Mode = (PinModeEnum)(int)root["extrPin0Mode"];
	config.extraPin1Mode = (PinModeEnum)(int)root["extrPin1Mode"];
	config.extraPin2Mode = (PinModeEnum)(int)root["extrPin2Mode"];
	config.extraPin3Mode = (PinModeEnum)(int)root["extrPin3Mode"];
	config.extraPin4Mode = (PinModeEnum)(int)root["extrPin4Mode"];

	saveConfig();

	controller.restart();

	String ret;
	return ret;
}

void loadConfig(void)
{
	DEBUG_PRINT("loadConfig\n");
	if (EEPROM.read(0) == 0xEE)
	{
		DEBUG_PRINT("CONFIG VALID!\n");
		EEPROM.get(1, config);
	}
	else
	{
		DEBUG_PRINT("CONFIG INVALID!!!\n");
		downloadConfigFlag = true;
	}
}

void downloadConfig(void)
{
	DEBUG_PRINT("downloadConfig\n");

	StaticJsonBuffer<200> jsonBuffer;
	JsonObject &out = jsonBuffer.createObject();

	out["messageUri"] = "/controller/panel";
	out["serialNumber"] = ESP.getChipId();
	out["classCode"] = (int)ClassPanel;
	out["action"] = (int)NetActionGet;

	Network.sendMessageToServer(out);
}

void saveConfig(void)
{
	DEBUG_PRINT("saveConfig\n");
	EEPROM.write(0, 0xEE);
	EEPROM.put(1, config);
	EEPROM.commit();
}

String netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	if (moduleIndex < config.totalModules)
	{
		modules[moduleIndex].netModuleCallback(action, json);
	}

	String ret;
	return ret;
}

String netControllerCallback(NetActionType action, const JsonObject &root)
{
	if (action == NetActionUpdate)
	{
		if (root.containsKey("extrPin0Mode"))
			controller.setExtraPin(0, (PinStateEnum)(int)root.containsKey("extrPin0Mode"));
		if (root.containsKey("extrPin1Mode"))
			controller.setExtraPin(1, (PinStateEnum)(int)root.containsKey("extrPin1Mode"));
		if (root.containsKey("extrPin2Mode"))
			controller.setExtraPin(2, (PinStateEnum)(int)root.containsKey("extrPin2Mode"));
		if (root.containsKey("extrPin3Mode"))
			controller.setExtraPin(3, (PinStateEnum)(int)root.containsKey("extrPin3Mode"));
		if (root.containsKey("extrPin4Mode"))
			controller.setExtraPin(4, (PinStateEnum)(int)root.containsKey("extrPin4Mode"));
	}
	// TODO: Consider returning the current state of all output pins
	String ret;
	return ret;
}

void serverReconnected(void)
{
	if (downloadConfigFlag)
	{
		downloadConfigFlag = false;
		downloadConfig();
	}
	else
	{
		DEBUG_PRINT("serverReconnected\n");
		for (byte x = 0; x < config.totalModules; x++)
		{
			StaticJsonBuffer<500> jsonBuffer;
			JsonObject &out = jsonBuffer.createObject();
			out["messageUri"] = "/controller/module";
			out["moduleIndex"] = x;
			out["class"] = (int)ClassPanel;
			out["action"] = (int)NetActionGet;
			Network.sendMessageToServer(out);
		}
	}
}
