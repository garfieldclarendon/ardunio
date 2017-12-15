#include "BlockDevice.h"
#include "NetworkManager.h"

BlockDevice::BlockDevice()
	: m_downloadConfig(false), m_currentState(BlockUnknown)
{
}


BlockDevice::~BlockDevice()
{
}

void BlockDevice::process(ModuleData &moduleData)
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

	if (getPort() < 8)
		moduleData.setByteA(data);
	else
		moduleData.setByteB(data);
}

void BlockDevice::setup(int deviceID, byte port)
{
	DEBUG_PRINT("BlockDevice::setup: deviceID: %d  Port: %d", deviceID, port);
	setID(deviceID);
	setPort(port);

	String json = loadConfig();
	DEBUG_PRINT("%s\n", json.c_str());
	if (json.length() == 0)
		m_downloadConfig = true;
	else
		m_downloadConfig = (parseConfig(json, false) == false);
}

void BlockDevice::processPin(byte pin, byte value)
{
	DEBUG_PRINT("PROCESS: pin: %d  DATA: %d\n", pin, value);
	if (pin == getPort())
	{
		DEBUG_PRINT(" MY PORT!  PROCESS: pin: %d  DATA: %d\n", pin, value);
		BlockState newState;
		if (value == PinOff)
			newState = BlockOccupied;
		else
			newState = BlockClear;
		if (m_currentState == newState)
		{
			m_currentState = newState;
			sendStatusMessage();
		}
	}
}


bool BlockDevice::parseConfig(String &jsonText, bool setVersion)
{
	DEBUG_PRINT("BlockDevice::parseConfig\n");
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

	if (setVersion)
	{
		jsonText = "";
		json.printTo(jsonText);
	}
	return true;
}

void BlockDevice::sendStatusMessage(void)
{
	DEBUG_PRINT("BlockDevice::sendStatusMessage: %d\n", m_currentState);

	UDPMessage message;
	message.setMessageID(BLK_STATUS);
	message.setID(getID());
	message.setField(0, m_currentState);
	NetManager.sendUdpMessage(message);
}

void BlockDevice::serverFound(void)
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
	sendStatusMessage();
}

void BlockDevice::processUDPMessage(ModuleData &moduleData, const UDPMessage &message)
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