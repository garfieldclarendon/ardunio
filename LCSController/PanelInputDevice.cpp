#include <FS.h>

#include "PanelInputDevice.h"
#include "NetworkManager.h"

extern StaticJsonBuffer<2048> jsonBuffer;

PanelInputDevice::PanelInputDevice()
	: m_downloadConfig(false)
{
	m_data.m_routeID = -1;
}


PanelInputDevice::~PanelInputDevice()
{
}

void PanelInputDevice::process(ModuleData &moduleData, UDPMessage &, byte &)
{
}

void PanelInputDevice::setup(int deviceID, byte port)
{
	setID(deviceID);
	setPort(port);

	String txt = loadConfig();
	if (txt.length() > 0)
	{
		m_downloadConfig = true;
		DEBUG_PRINT("PanelOutputDevice::setup: setting m_downloadConfig to TRUE\n");
	}
}

void PanelInputDevice::processUDPMessage(ModuleData &moduleData, const UDPMessage &message, UDPMessage &, byte &)
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
	if (m_data.m_routeID > 0 && value == PinOff && pin == getPort())
	{
		DEBUG_PRINT("PanelInputDevice::processPin PIN %d  VALUE %d  PORT %d  routeID %d\n", pin, value, getPort(), m_data.m_routeID);
		UDPMessage message;
		message.setMessageID(TRN_ACTIVATE_ROUTE);
		message.setID(m_data.m_routeID);
		NetManager.sendUdpMessage(message, true);
	}
}


bool PanelInputDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("PanelInputDevice::parseConfig\n");
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
	
	m_data.m_routeID = json["ROUTEID"];

	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	return true;
}

void PanelInputDevice::serverFound(UDPMessage &, byte &)
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

String PanelInputDevice::loadConfig(void)
{
	memset((void*)&m_data, 0, sizeof(PanelInputDataStruct));
	bool ret = false;
	String fileName("/Device_");
	fileName += getID();
	fileName += ".json";
	DEBUG_PRINT("PanelInputDevice::loadConfig %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "r");

	if (f)
	{
		f.read((uint8_t*)&m_data, sizeof(PanelInputDataStruct));
		f.close();
		if (m_data.m_version == CONFIG_VERSION)
			ret = true;
	}
	else
	{
		DEBUG_PRINT("PanelInputDevice Config file %s is missing or can not be opened\n", fileName.c_str());
	}
	if (ret)
		return "";
	else
		return "FAILED";
}

void PanelInputDevice::saveConfig(const String &)
{
	m_data.m_version = CONFIG_VERSION;
	String fileName("/Device_");
	fileName += getID();
	fileName += ".json";
	DEBUG_PRINT("SAVE PanelInputDevice Config %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		f.write((uint8_t*)&m_data, sizeof(PanelInputDataStruct));
		f.close();
	}
	else
	{
		DEBUG_PRINT("Error saving PanelInputDevice Config file %s\n", fileName.c_str());
	}
}