#include <FS.h>

#include "PanelOutputDevice.h"
#include "NetworkManager.h"
#include "DeviceMonitor.h"

extern StaticJsonBuffer<2048> jsonBuffer;

PanelOutputDevice::PanelOutputDevice()
	: m_downloadConfig(false)
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
	byte status = Devices.getDeviceStatus(m_data.m_itemID);
	if (status == m_data.m_onValue)
	{
		moduleData.writeBit(getPort(), HIGH);
	}
	else if (status == m_data.m_flashValue && m_data.m_flashValue > 0)
	{
		moduleData.setFlashOn(getPort());
	}
	else
	{
		moduleData.writeBit(getPort(), LOW);
	}
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
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLK_STATUS)
	{
		if (m_data.m_itemID == message.getID())
		{
			Devices.setDeviceStatus(m_data.m_itemID, message.getField(0));
		}
	}
	if (message.getMessageID() == DEVICE_STATUS)
	{
		byte index = 0;
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