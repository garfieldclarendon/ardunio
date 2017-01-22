#include <dummy.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <Wire.h>
#include <EEPROM.h>
#include <FS.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include "PanelModule.h"
#include "Controller.h"
#include "GlobalDefs.h"
#include "Structures.h"
#include "ConfigDownload.h"

#define MAX_ACTIVE_ROUTES 10
#define activeRouteTimeout 7000 
#define MAX_ACTIVE_ROUTE_RETRY 5

struct ActiveRoute
{
	long timeout;
	byte tryCount;
	PanelRouteStruct route;
};
typedef struct ActiveRoute ActiveRoute;

const int MODULE_CONFIG_BASE_ADDRESS = CONTROLLER_ID_ADDRESS + sizeof(int);
const int MODULE_ROUTE_BASE_ADDRESS = MODULE_CONFIG_BASE_ADDRESS + sizeof(PanelControllerConfigStruct);
const byte interruptPin = 14;

bool buttonPressed = false;
byte totalModules = 1;
byte totalRoutes = 0;

PanelModuleClass modules[MAX_PANEL_MODULES];
Controller controller(LocalServerPort);
PanelControllerConfigStruct *controllerConfig = NULL;
PanelControllerRouteConfigStruct *routeConfigDownload = NULL;
ActiveRoute activeRoutes[MAX_ACTIVE_ROUTES];

enum DownloadMode
{
	DownloadNone,
	DownloadModule,
	DownloadRoute
} downloadMode;

void messageCallback(const Message &message) 
{
	Message returnMessage;
	if (message.getMessageID() == SYS_CONFIG_CHANGED && (message.getControllerID() == 0 || message.getControllerID() == controller.getControllerID()))
	{
		downloadMode = DownloadModule;
		downloadConfig();
	}
	else if (totalRoutes == 0 && message.getMessageID() == SYS_HEARTBEAT)
	{
		downloadMode = DownloadRoute;
		downloadConfig();
	}
	else
	{
		for (byte x = 0; x < totalModules; x++)
		{
			if (message.getMessageID() == TRN_STATUS)
			{
				updateActiveRoutes(message.getIntValue1(), (TurnoutState)message.getByteValue1());
				updateActiveRoutes(message.getIntValue2(), (TurnoutState)message.getByteValue2());
			}
			returnMessage = modules[x].handleMessage(message);
			if (returnMessage.isValid())
				controller.sendNetworkMessage(message);
		}
	}
}

void setup() 
{
	Serial.begin(115200);
	Serial.println();
	Serial.println("setup");

	memset(&activeRoutes, 0, sizeof(ActiveRoute) * MAX_ACTIVE_ROUTES);
	EEPROM.begin(4096);

	bool result = SPIFFS.begin();
	Serial.println("SPIFFS opened: " + result);

	ConfigDownload.init(&controller);

	loadConfiguration();
	controller.setup(messageCallback, ClassPanel);

    // Add service to MDNS-SD
    // These are the services we want to hear FROM
    MDNS.addService("turnout", "tcp", LocalServerPort);
    MDNS.addService("block", "tcp", LocalServerPort);

	// If totalRoutes is less than 1, then this panel hasn't been configured yet.
	if (totalRoutes < 1)
		downloadConfig();

	setupHardware();
	for (byte x = 0; x < totalModules; x++)
	{
		modules[x].setup(x);
	}

	sendStatusMessage(false);

	Serial.println("setup complete");
}

void setupHardware(void)
{
	Serial.println("setup hardware");
	Wire.begin(); //creates a Wire object

	attachInterrupt(interruptPin, buttonPressedInterrupt, RISING);
}

void buttonPressedInterrupt(void)
{
	Serial.println("buttonPressedInterrupt");
	buttonPressed = true;
}

void loop() 
{
	controller.process();
	sendHeartbeatMessage();

	ConfigDownload.process();
	if (ConfigDownload.downloadComplete())
	{
		if (downloadMode == DownloadModule)
		{
			downloadMode = DownloadRoute;
			saveModuleConfig();
			ConfigDownload.reset();
			downloadConfig();
		}
		else
		{
			downloadMode = DownloadNone;
			saveRouteConfig();
			ConfigDownload.reset();
			loadConfiguration();
		}
	}

	Message message;
	bool flag = buttonPressed;
	buttonPressed = false;
	for (byte x = 0; x < totalModules; x++)
	{
		message = modules[x].process(flag);
		if (message.isValid())
		{
			if (message.getMessageID() == PANEL_ACTIVATE_ROUTE)
				addActiveRoute(message.getDeviceID());
			controller.sendNetworkMessage(message);
		}
	}
	checkActiveRoutes();
}

void addActiveRoute(int routeID)
{
	Serial.println("ADD ACTIVE ROUTE");
	for (byte x = 0; x < MAX_ACTIVE_ROUTES; x++)
	{
		if (activeRoutes[x].route.routeID == 0)
		{
			int routeIndex = RouteMap::getRouteIndex(routeID);
			if (routeIndex >= 0)
			{
				PanelRouteStruct route;
				String fileName;
				fileName = "Route_";
				fileName += routeID;
				fileName += ".dat";

				File f = SPIFFS.open(fileName,"r");
				if (f)
				{
					Serial.print("Reading route file: ");
					Serial.println(f.name());

					f.read((uint8_t*)&route, sizeof(PanelRouteStruct));
					activeRoutes[x].route = route;
					activeRoutes[x].timeout = millis();
					Serial.print("ADD ACTIVE ROUTE: ");
					Serial.println(route.routeID);
				}
				break;
			}
		}
	}
}

void updateActiveRoutes(int turnoutID, TurnoutState newState)
{
	for (byte x = 0; x < MAX_ACTIVE_ROUTES; x++)
	{
		if (activeRoutes[x].route.routeID > 0)
		{
			bool allAreSet = true;

			for (byte index = 0; index < MAX_ROUTE_ENTRIES; index++)
			{
				if (activeRoutes[x].route.entries[index].turnoutID == turnoutID && activeRoutes[x].route.entries[index].state == newState)
					activeRoutes[x].route.entries[index].turnoutID = 0;

				if (activeRoutes[x].route.entries[index].turnoutID > 0)
					allAreSet = false;
			}

			if (allAreSet)
			{
				Serial.print("REMOVE ACTIVE ROUTE: ");
				Serial.println(activeRoutes[x].route.routeID);
				activeRoutes[x].route.routeID = 0;
			}
		}
	}
}

void checkActiveRoutes(void)
{
	unsigned long t = millis();
	for (byte x = 0; x < MAX_ACTIVE_ROUTES; x++)
	{
		if (activeRoutes[x].route.routeID > 0 && t - activeRoutes[x].timeout > activeRouteTimeout)
		{
			Serial.println("Active Route timeout!  Sending route message");
			Message message;
			message.setDeviceID(activeRoutes[x].route.routeID);
			message.setMessageID(PANEL_ACTIVATE_ROUTE);
			message.setMessageClass(ClassRoute);
			controller.sendNetworkMessage(message);
			activeRoutes[x].timeout = t;
			activeRoutes[x].tryCount++;
			if (activeRoutes[x].tryCount >= MAX_ACTIVE_ROUTE_RETRY)
				activeRoutes[x].route.routeID = 0;
		}
	}
}

void loadConfiguration(void)
{
	if (controller.checkEEPROM(0xAD))
	{
		PanelControllerConfigStruct config;

		// Get the number of modules connected to this controller.
		// There's always at least one....the module built into the controller
		Serial.println("Getting the module configuration: ");
		memset(&config, 0, sizeof(PanelControllerConfigStruct));
		EEPROM.get(MODULE_CONFIG_BASE_ADDRESS, config);
		Serial.print("Setting each module's coinfiguration.  Total modules: ");
		Serial.println(config.mdouleCount);

		totalModules = config.mdouleCount;
		//The panel module has one module built in so make sure the totalModules is set to at least 1
		if (totalModules <= 0 || totalModules > 7)
			totalModules = 1;
		// Get the configuration information for each connected module
		Serial.print("Setting each module's coinfiguration.  Total modules: ");
		Serial.println(totalModules);
		for (byte x = 0; x < totalModules; x++)
		{
			modules[x].setConfiguration(config.moduleConfigs[x]);
		}
		loadRouteConfig();

		Serial.println("DONE GETTING CONFIGURATION");
	}
	else
	{
		Serial.println("EEPROM not set.  Initializing to 0");
		// This controller has not been configured yet or the configuration is no longer valid, 
		// write default values to memory
		PanelControllerConfigStruct config;

		totalModules = 1;
		totalRoutes = 0;
		memset(&config, 0, sizeof(PanelControllerConfigStruct));
		config.mdouleCount = 1;
		EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, config);
		// We just need to set the first byte of the PanelControllerRouteConfigStruct stored in EEPROM indicating that there's no route information stored
		EEPROM.put(MODULE_ROUTE_BASE_ADDRESS, totalRoutes);
		EEPROM.commit();
	}
}

void loadRouteConfig(void)
{
	Serial.println("Loading route configs");
	// Get all of the routeID's referenced by this controller.  The routes are too large to hold in memory so
	// we store just the routeID in an array.  The index of the routeID in the array indicates the index of the route in the EEPROM
	Serial.println("Getting the Panel Route configuration");
	RouteMap::init();
	EEPROM.get(MODULE_ROUTE_BASE_ADDRESS, totalRoutes);
	Serial.print("TOTAL ROUTES: ");
	Serial.println(totalRoutes);
	PanelRouteStruct route;
	Dir dir = SPIFFS.openDir("");
	while (dir.next()) 
	{
		File f = dir.openFile("r");
		if (f)
		{
			Serial.print("Reading route file: ");
			Serial.println(f.name());

			f.read((uint8_t*)&route, sizeof(PanelRouteStruct));
			Serial.print("RouteID: ");
			Serial.println(route.routeID);
			RouteMap::addRoute(route.routeID);
		}
	}	
}

void saveModuleConfig(void)
{
	Serial.println("Saving module configuration to EEPROM");
	if (controllerConfig != NULL)
	{
		Serial.println("Saving module configuration to EEPROM!!!!!!!");
		Serial.println(controllerConfig->mdouleCount);
		Serial.println(controllerConfig->moduleConfigs[0].inputs[0].id);

		EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, *controllerConfig);
		EEPROM.commit();
		delete controllerConfig;
		controllerConfig = NULL;
	}
}

void saveRouteConfig(void)
{
	Serial.println("Saving route configuration to SPIFFS");
	if (routeConfigDownload != NULL)
	{
		Serial.println("Saving route configuration to SPIFFS!!!!!!!");
		Serial.println(routeConfigDownload->count);
		Serial.println(routeConfigDownload->routes[0].routeID);
		Serial.println(MODULE_ROUTE_BASE_ADDRESS + sizeof(PanelControllerRouteConfigStruct));

		EEPROM.put(MODULE_ROUTE_BASE_ADDRESS, routeConfigDownload->count);
		EEPROM.commit();

		saveRoutesToFiles();
		totalRoutes = 0; 
		delete routeConfigDownload;
		routeConfigDownload = NULL;
		Serial.print("Saving route configuration to SPIFFS!!");
	}
}

void saveRoutesToFiles(void)
{
	for (int x = 0; x < routeConfigDownload->count; x++)
	{
		String fileName("Route_");
		fileName += routeConfigDownload->routes[x].routeID;
		fileName += ".dat";
		File f = SPIFFS.open(fileName, "w");

		if (f) 
		{
			Serial.print("Saving route: ");
			Serial.println(fileName);

			f.write((const uint8_t *)&routeConfigDownload->routes[x], sizeof(PanelRouteStruct));
			f.close();
		}
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

	ConfigDownload.reset();
	if (downloadMode != DownloadNone)
	{
		String key;
		key += ESP.getChipId(); 
		key += ",P,";
		key += downloadMode;

		if (downloadMode == DownloadModule)
		{
			controllerConfig = new PanelControllerConfigStruct;
			memset(controllerConfig, 0, sizeof(PanelControllerConfigStruct));
			ConfigDownload.downloadConfig((uint8_t *)controllerConfig, sizeof(PanelControllerConfigStruct), key);
		}
		else if (downloadMode == DownloadRoute)
		{
			routeConfigDownload = new PanelControllerRouteConfigStruct;
			memset(routeConfigDownload, 0, sizeof(PanelControllerRouteConfigStruct));
			ConfigDownload.downloadConfig((uint8_t *)routeConfigDownload, sizeof(PanelControllerRouteConfigStruct), key);
		}
	}
}

void sendStatusMessage(bool sendOnce)
{
	Serial.println("Sending status message");

	Message message;
	message.setMessageID(PANEL_STATUS);
	message.setControllerID(controller.getControllerID());
	message.setMessageClass(ClassPanel);

	controller.sendNetworkMessage(message, sendOnce);
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
