#include "SemaphoreDevice.h"

SemaphoreDevice::SemaphoreDevice()
	: SignalDevice(), m_motorBA(0)
{
	m_lockout = false;
}


SemaphoreDevice::~SemaphoreDevice()
{
}

void SemaphoreDevice::process(ModuleData &moduleData, UDPMessage &, byte &)
{
	UDPMessage outMessage;
	outMessage.setMessageID(DEVICE_STATUS);
	byte count = 0;
	m_lockout = false;
	SignalDevice::process(moduleData, outMessage, count);

	byte data = moduleData.getByteA();
	if (getGreenMode() == PinOn && getRedMode() == PinOff)
	{
//		DEBUG_PRINT("SET SEMAHORE TO GREEN\n\n");
		bitWrite(data, m_motorA, 0);
		bitWrite(data, m_motorB, 1);
	}
	else
	{
//		DEBUG_PRINT("SET SEMAHORE TO RED\n\n");
		bitWrite(data, m_motorA, 1);
		bitWrite(data, m_motorB, 0);
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
		m_downloadConfig = (SignalDevice::parseConfig(json, false) == false);

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
