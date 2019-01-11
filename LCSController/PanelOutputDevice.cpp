#include <FS.h>

#include "PanelOutputDevice.h"
#include "NetworkManager.h"
#include "DeviceMonitor.h"
#include "RouteMonitor.h"

extern StaticJsonBuffer<2048> jsonBuffer;

PanelOutputDevice::PanelOutputDevice()
	: m_downloadConfig(false), m_currentStatus(PinOff)
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
	bool updateStatus = false;
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLK_STATUS)
	{
		updateStatus = true;
		if (m_data.m_itemID == message.getID())
		{
			Devices.setDeviceStatus(m_data.m_itemID, message.getField(0));
		}
	}
	if (message.getMessageID() == DEVICE_STATUS)
	{
		byte index = 0;
		updateStatus = true;
		while (message.getDeviceID(index) > 0)
		{
			if (m_data.m_itemID == message.getDeviceID(index))
			{
				Devices.setDeviceStatus(m_data.m_itemID, message.getDeviceStatus(index));
			}
			index++;
		}
	}
	else if (message.getMessageID() == SYS_RESET_DEVICE_CONFIG)
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

	if (updateStatus)
	{
		updateCurrentStatus();
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
	// Panel LED's run in two modes.  If the ROUTEID entry is set in the configuration data (as indicated by m_itemIsRoute entry being set to true), 
	// then the LED follows the current state of the route.  In this case the m_onValue member contains either a 1 or a 0:
	//		1:  Turn the LED ON when the route is ACTIVE.  If any of the turnouts in the route are in the process of throwing, set the LED to flashing
	//		0:	Turn the LED ON when the route is NOT ACTIVE.  If any of the turnouts in the route are in the process of throwing, set the LED to ON
	//
	// If ROUTEID is not set, then the panel LED follows the state of a particular device (usually a turnout or a block occupancy detector).  In this case,
	// the ID of the device to monitor is in the m_itemID and should operate as follows:
	//		1.  If m_onValue matches the current state of the device, set the LED to ON
	//		2.  If m_flashValue matches the current state of the device, set the LED to FLASHING
	//		3.  Otherwise, the LED should be OFF.
	if (m_data.m_itemIsRoute == 1)
	{
		PinStateEnum routeState = Routes.getRouteState(m_data.m_itemID);
		// Turn the LED on if the route is active
		if (m_data.m_onValue == 1)
		{
			m_currentStatus = routeState;
		}
		else
		{
			// m_data.m_onValue == 0 means turn the LED on if the route is not active.  In this case, ignore the flashing state
			if (routeState == PinOn)
				m_currentStatus = PinOff;
			else
				m_currentStatus = PinOn;
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
		m_data.m_itemID = routeID;
		m_data.m_itemIsRoute = 1;
		JsonArray &turnouts = json["turnouts"].asArray();
		Routes.addRoute(routeID, turnouts);

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
}