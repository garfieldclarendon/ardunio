#include <FS.h>

#include "PanelOutputDevice.h"
#include "NetworkManager.h"
#include "DeviceMonitor.h"
#include "RouteMonitor.h"

extern StaticJsonBuffer<2048> jsonBuffer;

PanelOutputDevice::PanelOutputDevice()
	: m_downloadConfig(false), m_currentStatus(PinOff), m_routeList(nullptr)
{
	m_data.m_itemID = -1;
	m_data.m_onValue = 0;
	m_data.m_flashValue = 0;
}

PanelOutputDevice::~PanelOutputDevice()
{
}

void PanelOutputDevice::process(ModuleData &moduleData, UDPMessage &, byte &)
{
	//byte status = Devices.getDeviceStatus(m_data.m_itemID);
	//if (status == m_data.m_onValue)
	//{
	//	moduleData.writeBit(getPort(), HIGH);
	//}
	//else if (status == m_data.m_flashValue && m_data.m_flashValue > 0)
	//{
	//	moduleData.setFlashOn(getPort());
	//}
	//else
	//{
	//	moduleData.writeBit(getPort(), LOW);
	//}
	if (m_currentStatus == PinOn)
		moduleData.writeBit(getPort(), HIGH);
	else if (m_currentStatus == PinFlashing)
		moduleData.setFlashOn(getPort());
	else
		moduleData.writeBit(getPort(), LOW);
//	DEBUG_PRINT("PanelOutputDevice::process: AFTER %d\n", data);
}

void PanelOutputDevice::setup(int deviceID, byte port)
{
	setID(deviceID);
	setPort(port);

	String txt = loadConfig();
	if (txt.length() > 0)
	{
		m_downloadConfig = true;
		DEBUG_PRINT("PanelOutputDevice::setup: setting m_downloadConfig to TRUE\n");
	}
	else
	{
		Devices.addDevice(m_data.m_itemID);
	}
}

void PanelOutputDevice::processUDPMessage(ModuleData &, const UDPMessage &message, UDPMessage &, byte &)
{
	if (message.getMessageID() == TRN_STATUS || 
		message.getMessageID() == BLK_STATUS || 
		message.getMessageID() == DEVICE_STATUS)
	{
		updateCurrentStatus();
	}
	else if (message.getMessageID() == SYS_RESET_DEVICE_CONFIG || message.getMessageID() == SYS_DEVICE_CONFIG_CHANGED)
	{
		if (message.getID() == getID())
		{
			DEBUG_PRINT("processUDPMessage RESET DEVICE CONFIG\n", message.getID());
			String json = NetManager.getDeviceConfig(getID());
			parseConfig(json, true);
			saveConfig(json);
			setup(getID(), getPort());
		}
	}
}

PinStateEnum PanelOutputDevice::getPinState(void)
{
	PinStateEnum pinState(PinOff);
	byte status = Devices.getDeviceStatus(m_data.m_itemID);

	if (status > 0)
	{
		if (status == m_data.m_onValue)
			pinState = PinOn;
		else if (status == m_data.m_flashValue)
			pinState = PinFlashing;
	}
}

void PanelOutputDevice::updateCurrentStatus(void)
{
	// Panel LED's run in two modes.  If the ROUTEID, ROUTEID2, ROUTEID3... entries are set in the configuration data (as indicated by m_routeCount entry being greater than 0), 
	// then the LED follows the current state of one or more routes.  In this case the m_onValue member contains either a 1 or a 0:
	//		1:  Turn the LED ON when ANY specified route is ACTIVE.  If any of the turnouts in the route are in the process of throwing, set the LED to flashing
	//		0:	Turn the LED ON when ALL of the routes are NOT ACTIVE.  If any of the turnouts in the route(s) are in the process of throwing, set the LED to ON
	//
	// If ROUTEID is not set, then the panel LED follows the state of a particular device (usually a turnout or a block occupancy detector).  In this case,
	// the ID of the device to monitor is in the m_itemID and should operate as follows:
	//		1.  If m_onValue matches the current state of the device, set the LED to ON
	//		2.  If m_flashValue matches the current state of the device, set the LED to FLASHING
	//		3.  Otherwise, the LED should be OFF.
	if (m_data.m_routeCount > 0)
	{
		DEBUG_PRINT("PanelOutputDevice::updateCurrentStatus device: %d  ROUTE COUNT: %d\n", getID(), m_data.m_routeCount);
		PinStateEnum routeState;
		for (byte x = 0; x < m_data.m_routeCount; x++)
		{
			routeState = Routes.getRouteState(*m_routeList + x);
			DEBUG_PRINT("PanelOutputDevice::updateCurrentStatus device: %d  ROUTE PIN STATE: %d FOR ROUTE: %d\n", getID(), routeState, *m_routeList + x);
			// if onValue is 1 and the route is ACTIVE, break.
			// In this case, if the route is active or in the process of being active
			// we don't need to check the remaining routes.
			if (routeState != PinOff)
			{
				break;
			}
		}
		// Turn the LED on if the route is active
		if (m_data.m_onValue == 1)
		{
			m_currentStatus = routeState;
		}
		else
		{
			// m_data.m_onValue == 0 means turn the LED on if the route is not active.
			if (routeState == PinOn)
			{
				m_currentStatus = PinOff;
			}
			else
			{
				if(routeState == PinFlashing)
					m_currentStatus = PinOff;
				else
					m_currentStatus = PinOn;
			}
		}
	}
	else
	{
		byte status = Devices.getDeviceStatus(m_data.m_itemID);
		if (status == m_data.m_onValue)
		{
			m_currentStatus = PinOn;
		}
		else if (status == m_data.m_flashValue && m_data.m_flashValue > 0)
		{
			m_currentStatus = PinFlashing;
		}
		else
		{
			m_currentStatus = PinOff;
		}
	}
}

bool PanelOutputDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("PanelOutputDevice::parseConfig\n");
	jsonBuffer.clear();
	JsonObject &json = jsonBuffer.parseObject(jsonText);

	if (setVersion)
	{
		json["version"] = CONFIG_VERSION;
	}
	else if (json["version"] != (int)CONFIG_VERSION)
	{
		DEBUG_PRINT("parseConfig  WRONG VERSION.\n");
		return false;
	}

	m_data.m_itemID = json["ITEMID"];
	m_data.m_onValue = json["ONVALUE"];
	m_data.m_flashValue = json["FLASHINGVALUE"];
	int routeID = json["ROUTEID"];
	if (routeID > 0)
	{
		DEBUG_PRINT("parseConfig  ID: %d  FOUND ROUTES!  ROUTEID: %d.\n", getID(), routeID);
		int newRoutes[5];
		newRoutes[0] = routeID;
		m_data.m_routeCount = 1;
		JsonArray &turnouts = json["turnouts1"].asArray();
		Routes.addRoute(routeID, turnouts);
		byte counter = 2;
		byte routeCount = 1;
		String key;
		key = "ROUTEID";
		key += counter;
		DEBUG_PRINT("parseConfig  ID: %d  KEY: %s.\n", getID(), key.c_str());
		while (json.containsKey(key))
		{
			routeID = json[key];
			if (routeID > 0)
			{
				newRoutes[routeCount] = routeID;
				key = "turnouts";
				key += routeCount + 1;
				JsonArray &turnouts = json[key].asArray();
				DEBUG_PRINT("parseConfig  ID: %d  KEY: %s.\n", getID(), key.c_str());
				Routes.addRoute(routeID, turnouts);
				routeCount++;
			}
			counter++;
			key = "ROUTEID";
			key += counter;
		}
		m_data.m_routeCount = routeCount;
		m_routeList = new int[routeCount];
		memcpy(m_routeList, &newRoutes, routeCount);
	}

	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	return true;
}

void PanelOutputDevice::serverFound(UDPMessage &, byte &)
{
	if (m_downloadConfig)
	{
		String json = NetManager.getDeviceConfig(getID());
		if (json.length() > 0)
		{
			parseConfig(json, true);
			saveConfig(json);
			m_downloadConfig = false;
			setup(getID(), getPort());
		}
	}
}

String PanelOutputDevice::loadConfig(void)
{
	memset((void*)&m_data, 0, sizeof(PanelOutputDataStruct));
	bool ret = false;
	String fileName("/Device_");
	fileName += getID();
	fileName += ".json";
	DEBUG_PRINT("PanelOutputDevice::loadConfig %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "r");

	if (f)
	{
		f.read((uint8_t*)&m_data, sizeof(PanelOutputDataStruct));
		f.close();
		if (m_data.m_version == CONFIG_VERSION)
			ret = true;
	}
	else
	{
		DEBUG_PRINT("PanelOutputDevice Config file %s is missing or can not be opened\n", fileName.c_str());
	}

	if (ret == true && m_data.m_routeCount > 0)
	{
		m_routeList = new int[m_data.m_routeCount];
		String fileName("/Device_");
		fileName += getID();
		fileName += "_routes.json";
		DEBUG_PRINT("PanelOutputDevice::loadConfig routes %s\n", fileName.c_str());

		File f = SPIFFS.open(fileName, "r");

		if (f)
		{
			f.read((uint8_t*)m_routeList, m_data.m_routeCount);
			f.close();
		}
		else
		{
			DEBUG_PRINT("PanelOutputDevice route list file %s is missing or can not be opened\n", fileName.c_str());
		}
	}

	updateCurrentStatus();

	if (ret)
		return "";
	else
		return "FAILED";
}

void PanelOutputDevice::saveConfig(const String &)
{
	m_data.m_version = CONFIG_VERSION;
	String fileName("/Device_");
	fileName += getID();
	fileName += ".json";
	DEBUG_PRINT("SAVE PanelOutputDevice Config %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		f.write((uint8_t*)&m_data, sizeof(PanelOutputDataStruct));
		f.close();
	}
	else
	{
		DEBUG_PRINT("Error saving PanelOutputDevice Config file %s\n", fileName.c_str());
	}

	// Load routes if they exist
	if (m_data.m_routeCount > 0)
	{
		String fileName("/Device_");
		fileName += getID();
		fileName += "_routes.json";
		DEBUG_PRINT("SAVE PanelOutputDevice Route List %s\n", fileName.c_str());

		File f = SPIFFS.open(fileName, "w");

		if (f)
		{
			f.write((uint8_t*)m_routeList, m_data.m_routeCount);
			f.close();
		}
		else
		{
			DEBUG_PRINT("Error saving PanelOutputDevice route list file %s\n", fileName.c_str());
		}
	}
}