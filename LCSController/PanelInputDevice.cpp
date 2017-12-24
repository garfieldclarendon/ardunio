#include "PanelInputDevice.h"
#include "NetworkManager.h"

PanelInputDevice::PanelInputDevice()
	: m_downloadConfig(false), m_routeID(-1)
{
}


PanelInputDevice::~PanelInputDevice()
{
}

void PanelInputDevice::process(ModuleData &moduleData)
{
}

void PanelInputDevice::setup(int deviceID, byte port)
{
	setID(deviceID);
	setPort(port);

	String json = loadConfig();
	DEBUG_PRINT("%s\n", json.c_str());
	if (json.length() == 0)
		m_downloadConfig = true;
	else
		m_downloadConfig = (parseConfig(json, false) == false);
}

void PanelInputDevice::processUDPMessage(ModuleData &moduleData, const UDPMessage &message)
{
	if (message.getMessageID() == SYS_RESET_DEVICE_CONFIG)
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

void PanelInputDevice::processPin(byte pin, byte value)
{
	if (m_routeID > 0 && value == PinOff && pin == getPort())
	{
		DEBUG_PRINT("PanelInputDevice::processPin PIN %d  VALUE %d  PORT %d  routeID %d\n", pin, value, getPort(), m_routeID);
		UDPMessage message;
		message.setMessageID(TRN_ACTIVATE_ROUTE);
		message.setID(m_routeID);
		NetManager.sendUdpMessage(message, true);
	}
}


bool PanelInputDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("PanelInputDevice::parseConfig\n");
	StaticJsonBuffer<1024> jsonBuffer;
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
	
	m_routeID = json["ROUTEID"];

	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	return true;
}

void PanelInputDevice::serverFound(void)
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
