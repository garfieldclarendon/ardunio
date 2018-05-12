#include <FS.h>

#include "Device.h"
#include "NetworkManager.h"

Device::Device()
	: m_id(-1), m_port(0)
{
}

Device::~Device()
{
}

String Device::loadConfig(void)
{
	DEBUG_PRINT("LOAD DEVICE CONFIGURATION\n");
	String json;
	String fileName("/Device_");
	fileName += getID();
	fileName += ".json";
	File f = SPIFFS.open(fileName, "r");

	if (f)
	{
		DEBUG_PRINT("Reading Device config: %s\n", fileName.c_str());

		json = f.readString();
		f.close();
	}
	else
	{
		DEBUG_PRINT("Configuration file %s is missing or can not be opened\n", fileName.c_str());
	}
	return json;
}

void Device::downloadConfig(void)
{
	String json = NetManager.getDeviceConfig(getID());
	if (json.length() > 0)
		saveConfig(json);
}

void Device::saveConfig(const String &json)
{
	String fileName("/Device_");
	fileName += getID();
	fileName += ".json";

	if (SPIFFS.exists(fileName))
		SPIFFS.remove(fileName);
	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		DEBUG_PRINT("Saving Device config: %s\n", fileName.c_str());
		DEBUG_PRINT("Saving Device config DATA: \n%s\n", json.c_str());

		f.write((const uint8_t *)json.c_str(), json.length());
		f.close();
	}
	else
	{
		DEBUG_PRINT("Saving Device config FAILED: %s  COULD NOT OPEN FILE\n", fileName.c_str());
	}
}
