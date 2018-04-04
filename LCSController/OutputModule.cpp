#include "OutputModule.h"
#include "NetworkManager.h"
#include "Device.h"

OutputModule::OutputModule(void)
	: m_outputA(0), m_outputB(0), m_currentTimeout(0), m_blinkingTimeout(250), m_flash(true)
{
}

void OutputModule::setupWire(byte address)
{
	setAddress(address);
	m_outputA = 0;
	m_outputB = 0;

	DEBUG_PRINT("setup OutputModule: %d\n", address);

	// set ports A & B to output
	m_expander.setAddress(address);
	m_expander.expanderBegin(0, 0);
	delay(100);
	m_expander.writeA(m_outputA);
	m_expander.writeB(m_outputB);
	DEBUG_PRINT("OutputModule::setupWire:  address %d DONE!!!!\n", address);
}

void OutputModule::processWire(void)
{
	byte previousOutputA = m_outputA;
	byte previousOutputB = m_outputB;
	ModuleData data(m_outputA, m_outputB);

	UDPMessage outMessage;
	byte count = 0;
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			device->process(data, outMessage, count);
		}
	}

	setFlashingPins(data);

	m_outputA = data.getByteA();
	m_outputB = data.getByteB();

	if (m_outputA != previousOutputA)
	{
//		DEBUG_PRINT("OutputModule::processWire:  address %d SEND PORT A %d\n", getAddress(), m_outputA);
		m_expander.writeA(m_outputA);
	}
	if (m_outputB != previousOutputB)
	{
//		DEBUG_PRINT("OutputModule::processWire:  address %d SEND PORT B %d\n", getAddress(), m_outputB);
		m_expander.writeB(m_outputB);
	}
}

void OutputModule::setFlashingPins(ModuleData &data)
{
	unsigned long t = millis();
	if ((t - m_currentTimeout) > m_blinkingTimeout)
	{
		m_currentTimeout = t;
		m_flash = !m_flash;

		for (byte x = 0; x < MAX_DEVICES; x++)
		{
			if (data.isFlashSet(x))
			{
				data.writeBit(x, m_flash);
			}
		}
	}
}

void OutputModule::processUDPMessageWire(const UDPMessage &message)
{
	UDPMessage outMessage;
	outMessage.setMessageID(DEVICE_STATUS);
	byte count = 0;
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			ModuleData moduleData;
			device->processUDPMessage(moduleData, message, outMessage, count);
		}
	}
}