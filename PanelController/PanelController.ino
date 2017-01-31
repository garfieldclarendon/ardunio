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
				for (byte x = 0; x < MAX_MODULES; x++)
				{
					if (message.getDeviceStatusID(x) == 0)
						break;
					updateActiveRoutes(message.getDeviceStatusID(x), (TurnoutState)message.getDeviceStatus(x));
				}
			}
			returnMessage = modules[x].handleMessage(message);
			if (returnMessage.isValid())
				controller.sendNetworkMessage(message);
		}
	}
}

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

	memset(&activeRoutes, 0, sizeof(ActiveRoute) * MAX_ACTIVE_ROUTES);
	EEPROM.begin(4096);

	bool result = SPIFFS.begin();
	DEBUG_PRINT("SPIFFS opened: %d\n", result);

	ConfigDownload.init(&controller);

	loadConfiguration();
	controller.setup(messageCallback, ClassPanel);

	// If totalRoutes is less than 1, then this panel hasn't been configured yet.
	if (totalRoutes < 1)
		downloadConfig();

	setupHardware();
	for (byte x = 0; x < totalModules; x++)
	{
		modules[x].setup(x);
	}

	sendStatusMessage(false);

	DEBUG_PRINT("setup complete\n");
}

void setupHardware(void)
{
	DEBUG_PRINT("setup hardware\n");
	Wire.begin(); //creates a Wire object

	attachInterrupt(interruptPin, buttonPressedInterrupt, RISING);
}

void buttonPressedInterrupt(void)
{
	DEBUG_PRINT("buttonPressedInterrupt\n");
	buttonPressed = true;
}

void loop() 
{
	controller.process();

	if (controller.getWiFiReconnected())
	{
		Message message;
		message.setMessageID(SYS_REQEST_STATUS);
		message.setControllerID(controller.getControllerID());
		message.setMessageClass(ClassPanel);

		controller.sendNetworkMessage(message, true);
	}

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
	DEBUG_PRINT("ADD ACTIVE ROUTE\n");
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
					DEBUG_PRINT("Reading route file: %s\n", f.name());

					f.read((uint8_t*)&route, sizeof(PanelRouteStruct));
					activeRoutes[x].route = route;
					activeRoutes[x].timeout = millis();
					activeRoutes[x].tryCount = 0;
					DEBUG_PRINT("ADD ACTIVE ROUTE: %d\n", route.routeID);
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
				DEBUG_PRINT("REMOVE ACTIVE ROUTE: %d\n", activeRoutes[x].route.routeID);
				clearActiveRouteEntry(x);
			}
		}
	}
}

void clearActiveRouteEntry(byte index)
{
	activeRoutes[index].route.routeID = 0;
	activeRoutes[index].tryCount = 0;
	activeRoutes[index].timeout = 0;
}

void checkActiveRoutes(void)
{
	unsigned long t = millis();
	for (byte x = 0; x < MAX_ACTIVE_ROUTES; x++)
	{
		if (activeRoutes[x].route.routeID > 0 && t - activeRoutes[x].timeout > activeRouteTimeout)
		{
			DEBUG_PRINT("Active Route timeout!  Sending route message\n");
			Message message;
			message.setDeviceID(activeRoutes[x].route.routeID);
			message.setMessageID(PANEL_ACTIVATE_ROUTE);
			message.setMessageClass(ClassRoute);
			controller.sendNetworkMessage(message);
			activeRoutes[x].timeout = t;
			activeRoutes[x].tryCount++;
			if (activeRoutes[x].tryCount >= MAX_ACTIVE_ROUTE_RETRY)
				clearActiveRouteEntry(x);
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
		DEBUG_PRINT("Getting the module configuration: \n");
		memset(&config, 0, sizeof(PanelControllerConfigStruct));
		EEPROM.get(MODULE_CONFIG_BASE_ADDRESS, config);
		DEBUG_PRINT("Setting each module's coinfiguration.  Total modules: %d\n", config.mdouleCount);

		totalModules = config.mdouleCount;
		//The panel module has one module built in so make sure the totalModules is set to at least 1
		if (totalModules <= 0 || totalModules > 7)
			totalModules = 1;
		// Get the configuration information for each connected module
		DEBUG_PRINT("Setting each module's coinfiguration.  Total modules: %d\n", totalModules);
		for (byte x = 0; x < totalModules; x++)
		{
			modules[x].setConfiguration(config.moduleConfigs[x]);
		}
		loadRouteConfig();

		DEBUG_PRINT("DONE GETTING CONFIGURATION\n");
	}
	else
	{
		DEBUG_PRINT("EEPROM not set.  Initializing to 0\n");
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
	DEBUG_PRINT("Loading route configs\n");
	// Get all of the routeID's referenced by this controller.  The routes are too large to hold in memory so
	// we store just the routeID in an array.  The index of the routeID in the array indicates the index of the route in the EEPROM
	DEBUG_PRINT("Getting the Panel Route configuration\n");
	RouteMap::init();
	EEPROM.get(MODULE_ROUTE_BASE_ADDRESS, totalRoutes);
	DEBUG_PRINT("TOTAL ROUTES: %d\n", totalRoutes);
	PanelRouteStruct route;
	Dir dir = SPIFFS.openDir("");
	while (dir.next()) 
	{
		File f = dir.openFile("r");
		if (f)
		{
			DEBUG_PRINT("Reading route file: %s\n", f.name());

			f.read((uint8_t*)&route, sizeof(PanelRouteStruct));
			DEBUG_PRINT("RouteID: %d\n", route.routeID);
			RouteMap::addRoute(route.routeID);
		}
	}	
}

void saveModuleConfig(void)
{
	DEBUG_PRINT("Saving module configuration to EEPROM\n");
	if (controllerConfig != NULL)
	{
		DEBUG_PRINT("Saving module configuration to EEPROM!!!!!!!\nTotal Modules: %d\n", controllerConfig->mdouleCount);

		EEPROM.put(MODULE_CONFIG_BASE_ADDRESS, *controllerConfig);
		EEPROM.commit();
		delete controllerConfig;
		controllerConfig = NULL;
	}
}

void saveRouteConfig(void)
{
	DEBUG_PRINT("Saving route configuration to SPIFFS\n");
	if (routeConfigDownload != NULL)
	{
		DEBUG_PRINT("Saving route configuration to SPIFFS!!!!!!!\nCount: %d\n", routeConfigDownload->count);

		EEPROM.put(MODULE_ROUTE_BASE_ADDRESS, routeConfigDownload->count);
		EEPROM.commit();

		saveRoutesToFiles();
		totalRoutes = 0; 
		delete routeConfigDownload;
		routeConfigDownload = NULL;
		DEBUG_PRINT("Done saving route configuration to SPIFFS!!\n");
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
			DEBUG_PRINT("Saving route: %s\n", fileName.c_str());

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
	DEBUG_PRINT("Sending status message\n");

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
