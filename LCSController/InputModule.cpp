#include "InputModule.h"
#include "NetworkManager.h"
#include "Device.h"

InputModule::InputModule(void)
	: m_inputA(0), m_inputB(0)
{
}

void InputModule::setupWire(byte address)
{
	setAddress(address);
	m_inputA = 0;
	m_inputB = 0;

	DEBUG_PRINT("InputModule: Setup InputModule: %d\n", address);

	// set ports A & B to input
	m_expander.setAddress(address);
	m_expander.expanderBegin(0xFF, 0xFF);
	DEBUG_PRINT("InputModule::setupWire:  address %d DONE!!!!\n", address);
}

void InputModule::processWire(void)
{
	// Read and process Port A
	byte input = m_expander.readA();

	if (input != m_inputA)
	{
		DEBUG_PRINT("InputModule::processA  Module Address: %d\nPin values: %02x\n", getAddress(), input);

		for (byte index = 0; index < 8; index++)
		{
			if (bitRead(m_inputA, index) != bitRead(input, index))
			{
				if (bitRead(input, index) == HIGH)
					handleInput(index, PinOn);
				else
					handleInput(index, PinOff);
			}
		}
		m_inputA = input;
	}
	// Read and process Port B
	input = m_expander.readB();
	if (input != m_inputB)
	{
		DEBUG_PRINT("InputModule::processB  Module Address: %d\nPin values: %02x\n", getAddress(), input);

		for (byte index = 0; index < 8; index++)
		{
			if (bitRead(m_inputB, index) != bitRead(input, index))
			{
				if (bitRead(input, index) == HIGH)
					handleInput(index + 8, PinOn);
				else
					handleInput(index + 8, PinOff);
			}
		}
		m_inputB = input;
	}
}

void InputModule::finishSetupWire(void)
{
	DEBUG_PRINT("InputModule::finishSetupWire  Module Address: %d\n", getAddress());
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			PinStateEnum pinState;
			byte pin;
			if (device->getPort() < 8)
				pinState = (PinStateEnum)bitRead(m_inputA, device->getPort());
			else
				pinState = (PinStateEnum)bitRead(m_inputB, device->getPort() - 8);

			device->processPin(device->getPort(), pinState);
		}
	}
}

void InputModule::handleInput(byte pin, byte pinState)
{
	DEBUG_PRINT("InputModule::handleInput  Module Address: %d\nPin index: %d\n", getAddress(), pin);
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		Device *device = getDevice(x);
		if (device && device->getPort() == pin)
			device->processPin(pin, pinState);
	}
}

void InputModule::processUDPMessageWire(const UDPMessage &message)
{
	byte dataA(m_inputA);
	byte dataB(m_inputB);
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

	if (m_inputA != dataA || m_inputB != dataB)
	{
//		DEBUG_PRINT("processUDPMessage:  CURRENT_STATE %d != %d\n", m_currentState, data);
		m_inputA = dataA;
		m_inputB = dataB;
		m_expander.writeA(m_inputA);
		m_expander.writeB(m_inputB);
	}
}
