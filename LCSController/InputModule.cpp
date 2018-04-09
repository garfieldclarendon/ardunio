#include "InputModule.h"
#include "NetworkManager.h"
#include "Device.h"
#include "GlobalDefs.h"
#include "BlockDevice.h"

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
	byte inputb = m_expander.readB();

	ModuleData moduleData(input, inputb);

	UDPMessage outMessage;
	outMessage.setMessageID(DEVICE_STATUS);
	outMessage.setID(getAddress());
	byte count = 0;
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		if (count >= 8)
		{
			NetManager.sendUdpMessage(outMessage, true);
			outMessage = UDPMessage();
			outMessage.setMessageID(DEVICE_STATUS);
			outMessage.setID(getAddress());
			count = 0;
		}

		Device *device = getDevice(x);
		if (device && device->getDeviceType() == DeviceBlock)
			device->process(moduleData, outMessage, count);
	}
	if (count > 0)
	{
		NetManager.sendUdpMessage(outMessage, true);
	}

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
	input = inputb;
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

	UDPMessage outMessage;
	outMessage.setMessageID(DEVICE_STATUS);
	outMessage.setID(getAddress());
	byte count = 0;
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		if (count >= 8)
		{
			NetManager.sendUdpMessage(outMessage, true);
			outMessage = UDPMessage();
			outMessage.setMessageID(DEVICE_STATUS);
			outMessage.setID(getAddress());
			count = 0;
		}
		Device *device = getDevice(x);
		if (device)
		{
			byte input = m_expander.readA();
			byte inputb = m_expander.readB();

			ModuleData moduleData(input, inputb);
			device->process(moduleData, outMessage, count);
		}
	}
	if (count >= 8)
	{
		NetManager.sendUdpMessage(outMessage, true);
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
	byte dataA;
	byte dataB;
	dataA = m_expander.readA();
	dataB = m_expander.readB();

	ModuleData moduleData;
	moduleData.setByteA(dataA);
	moduleData.setByteB(dataB);

	UDPMessage outMessage;
	outMessage.setMessageID(DEVICE_STATUS);
	outMessage.setID(getAddress());
	byte count = 0;
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		if (count > 0)
		{
			NetManager.sendUdpMessage(outMessage, true);
			outMessage = UDPMessage();
			outMessage.setMessageID(DEVICE_STATUS);
			outMessage.setID(getAddress());
			count = 0;
		}

		Device *device = getDevice(x);
		if (device && device->getDeviceType() == DeviceBlock)
		{
			device->processUDPMessage(moduleData, message, outMessage, count);
		}
	}
	if (count > 0)
	{
		NetManager.sendUdpMessage(outMessage, true);
	}
}

void InputModule::sendStatusMessage(void)
{
	DEBUG_PRINT("InputModule::sendStatusMessage  Module Address: %d\n\n", getAddress());
	UDPMessage outMessage;
	outMessage.setMessageID(DEVICE_STATUS);
	outMessage.setID(getAddress());

	byte count = 0;
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		if (count >= 8)
		{
			NetManager.sendUdpMessage(outMessage, true);
			outMessage = UDPMessage();
			outMessage.setMessageID(DEVICE_STATUS);
			outMessage.setID(99 - getAddress());
			count = 0;
		}

		Device *device = getDevice(x);
		if (device)
		{
			if (device->getDeviceType() == DeviceBlock)
			{
				outMessage.setDeviceID(count, device->getID());
				outMessage.setDeviceStatus(count, device->getCurrentStatus());
				count++;
			}
		}
	}
	if (count > 0)
		NetManager.sendUdpMessage(outMessage, true);
//	DEBUG_PRINT("InputModule::sendStatusMessage  Module Address: %d DONE\n\n", getAddress());
}