#include "OutputModule.h"
#include "NetworkManager.h"
#include "Device.h"

OutputModule::OutputModule(void)
	: m_outputA(0), m_outputB(0)
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

	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			ModuleData data(m_outputA, m_outputB);
			device->process(data);
			m_outputA = data.getByteA();
			m_outputB = data.getByteB();
		}
	}

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

void OutputModule::processUDPMessageWire(const UDPMessage &message)
{
	byte dataA(m_outputA);
	byte dataB(m_outputB);
	dataA = m_expander.readA();
	dataB = m_expander.readB();

	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			ModuleData moduleData;
			moduleData.setByteA(dataA);
			moduleData.setByteB(dataB);

			device->processUDPMessage(moduleData, message);
			dataA = moduleData.getByteA();
			dataB = moduleData.getByteB();
		}
	}

	if (m_outputA != dataA || m_outputB != dataB)
	{
//		DEBUG_PRINT("processUDPMessageWire:  CURRENT_STATE %d != %d\n", m_outputA, dataA);
		m_outputA = dataA;
		m_outputB = dataB;
		m_expander.writeA(m_outputA);
		m_expander.writeB(m_outputB);
	}
}