#include "SemaphoreDevice.h"

SemaphoreDevice::SemaphoreDevice()
	: m_downloadConfig(false), m_motorBA(0)
{
}


SemaphoreDevice::~SemaphoreDevice()
{
}

void SemaphoreDevice::process(ModuleData &moduleData)
{
	SignalDevice::process(moduleData);

	byte data = moduleData.getByteA();
	if (getGreenMode() == PinOn)
	{
		bitWrite(data, m_motorA, 1);
		bitWrite(data, m_motorB, 0);
	}
	else
	{
		bitWrite(data, m_motorA, 0);
		bitWrite(data, m_motorB, 1);
	}
	moduleData.setByteA(data);
}

void SemaphoreDevice::setup(int deviceID, byte port)
{
	setID(deviceID);
	setPort(port);

	String json = loadConfig();
	DEBUG_PRINT("%s\n", json.c_str());
	if (json.length() == 0)
		m_downloadConfig = true;
	else
		m_downloadConfig = (parseConfig(json, false) == false);

	if (port == 0)
	{
		m_motorA = m_motorBA == 1 ? 1 : 0;
		m_motorB = m_motorBA == 1 ? 0 : 1;
	}
	else
	{
		m_motorA = m_motorBA == 1 ? 5 : 4;
		m_motorB = m_motorBA == 1 ? 4 : 5;
	}
}

bool SemaphoreDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("TurnoutDevice::parseConfig\n");
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

	if (json.containsKey("MOTORPIN"))
		m_motorBA = (int)json["MOTORPIN"];

	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	DEBUG_PRINT("TurnoutDevice::parseConfig: %d MOTORPIN %d\n", getID(), m_motorBA);
	return true;
}
