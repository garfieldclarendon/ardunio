#include "PanelOutputDevice.h"
#include "NetworkManager.h"

PanelOutputDevice::PanelOutputDevice()
	: m_downloadConfig(false), m_itemID(-1), m_onValue(0), m_flashValue(0), m_currentValue(0)
{
}

PanelOutputDevice::~PanelOutputDevice()
{
}

void PanelOutputDevice::process(ModuleData &moduleData, UDPMessage &, byte &)
{
	if (m_currentValue == m_onValue)
	{
		moduleData.writeBit(getPort(), HIGH);
	}
	else if (m_currentValue == m_flashValue)
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

	String json = loadConfig();
	DEBUG_PRINT("%s\n", json.c_str());
	if (json.length() == 0)
		m_downloadConfig = true;
	else
		m_downloadConfig = (parseConfig(json, false) == false);
}

void PanelOutputDevice::processUDPMessage(ModuleData &, const UDPMessage &message, UDPMessage &, byte &)
{
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLK_STATUS)
	{
		if (m_itemID == message.getID())
		{
			m_currentValue = message.getField(0);
		}
	}
	if (message.getMessageID() == DEVICE_STATUS)
	{
		byte index = 0;
		while (message.getDeviceID(index) > 0)
		{
			if (m_itemID == message.getDeviceID(index))
			{
				m_currentValue = message.getDeviceStatus(index);
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
	if (m_currentValue == m_onValue)
		pinState = PinOn;
	else if (m_currentValue == m_flashValue)
		pinState = PinFlashing;
}

bool PanelOutputDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("PanelOutputDevice::parseConfig\n");
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

	m_itemID = json["ITEMID"];
	m_onValue = json["ONVALUE"];
	m_flashValue = json["FLASHINGVALUE"];

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
