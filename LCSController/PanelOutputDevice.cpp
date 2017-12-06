#include "PanelOutputDevice.h"
#include "NetworkManager.h"

PanelOutputDevice::PanelOutputDevice()
	: m_downloadConfig(false), m_itemID(-1), m_onValue(0), m_flashValue(0), m_currentValue(0), m_currentBlinkTimeout(0), m_blinkingTimeout(250)
{
}

PanelOutputDevice::~PanelOutputDevice()
{
}

void PanelOutputDevice::process(ModuleData &moduleData)
{
	byte data;
	byte pin;
	if (getPort() < 8)
	{
		data = moduleData.getByteA();
		pin = getPort();
	}
	else
	{
		data = moduleData.getByteB();
		pin = getPort() - 8;
	}
//	DEBUG_PRINT("PanelOutputDevice::process: BEFORE %d\n", data);

	if (m_currentValue == m_onValue)
	{
		bitWrite(data, pin, HIGH);
	}
	else if (m_currentValue == m_flashValue)
	{
		static bool flash = true;
		unsigned long t = millis();
		if (t - m_currentBlinkTimeout > m_blinkingTimeout)
		{
			m_currentBlinkTimeout = t;
			bitWrite(data, pin, flash);
			flash = !flash;
		}
	}
	else
	{
		bitWrite(data, pin, LOW);
	}

//	DEBUG_PRINT("PanelOutputDevice::process: AFTER %d\n", data);
	if (getPort() < 8)
		moduleData.setByteA(data);
	else
		moduleData.setByteB(data);
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

void PanelOutputDevice::processUDPMessage(ModuleData &moduleData, const UDPMessage &message)
{
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLK_STATUS)
	{
		if (m_itemID == message.getID())
		{
			m_currentValue = message.getField(0);
			process(moduleData);
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

void PanelOutputDevice::serverFound(void)
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
