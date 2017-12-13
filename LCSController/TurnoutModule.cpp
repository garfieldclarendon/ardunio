#include <Arduino.h>

#include "TurnoutModule.h"
#include "NetworkManager.h"
#include "Device.h"
#include "ModuleData.h"
#include "TurnoutDevice.h"

TurnoutModule::TurnoutModule(void)
	: m_currentState(0)
{
}

byte TurnoutModule::getIODirConfig(void) const
{
	byte iodir = 0;

	bitWrite(iodir, motorAPin1, 0);
	bitWrite(iodir, motorBPin1, 0);
	bitWrite(iodir, motorAPin2, 0);
	bitWrite(iodir, motorBPin2, 0);
	bitWrite(iodir, feedbackAPin1, 1);
	bitWrite(iodir, feedbackBPin1, 1);
	bitWrite(iodir, feedbackAPin2, 1);
	bitWrite(iodir, feedbackBPin2, 1);

	return iodir;
}

void TurnoutModule::setupNoWire(void)
{
	DEBUG_PRINT("TurnoutModule::setupNoWire\n");
	m_currentState = 0;

	byte pinMap[8];
	pinMap[motorAPin1] = motor1_pinA;
	pinMap[motorBPin1] = motor1_pinB;
	pinMap[motorAPin2] = motor2_pinA;
	pinMap[motorBPin2] = motor2_pinB;
//	pinMap[feedbackAPin1] = normal2_pin;
//	pinMap[feedbackBPin1] = diverge2_pin;
//	pinMap[feedbackAPin2] = normal1_pin;
//	pinMap[feedbackBPin2] = diverge1_pin;

	pinMap[feedbackAPin1] = normal1_pin;
	pinMap[feedbackBPin1] = diverge1_pin;
	pinMap[feedbackAPin2] = normal2_pin;
	pinMap[feedbackBPin2] = diverge2_pin;

	DEBUG_PRINT("TurnoutModule::setup  SETTING PINMODE\n");
	m_noWireHandler.setNoWirePinMap(pinMap);
	m_noWireHandler.setNoWirePinDir(getIODirConfig());
}

void TurnoutModule::setupWire(byte address)
{
	DEBUG_PRINT("TurnoutModule::setupWire  Address: %d\n", address);
	setAddress(address);

	m_currentState = 0;
	// Force the diverging pin ON so that it resets to normal
	// if the turnout happens to be set to the diverging route
	bitWrite(m_currentState, feedbackAPin1, 1);
	bitWrite(m_currentState, feedbackBPin1, 0);
	bitWrite(m_currentState, feedbackAPin2, 1);
	bitWrite(m_currentState, feedbackBPin2, 0);

	byte iodir = getIODirConfig();
	m_expander.setAddress(address);
	m_expander.expanderBegin(iodir);
}

void TurnoutModule::finishSetupWire(void)
{
	byte data = 0;
	for (byte x = 0; x < MAX_TURNOUTS; x++)
	{
		TurnoutDevice *device = static_cast<TurnoutDevice *>(getDevice(x));
		if (device)
		{
			device->setTurnout(data, TrnNormal);
		}
	}
	m_expander.write(data);
}

void TurnoutModule::finishSetupNoWire(void)
{
	byte data = 0;
	for (byte x = 0; x < MAX_TURNOUTS; x++)
	{
		TurnoutDevice *device = static_cast<TurnoutDevice *>(getDevice(x));
		if (device)
		{
			device->setTurnout(data, TrnNormal);
		}
	}
	m_noWireHandler.writeNoWire(data);
}


void TurnoutModule::processNoWire(void)
{
	byte data(m_currentState);
	m_noWireHandler.readNoWire(data);
//	DEBUG_PRINT("processNoWire:  CURRENT_STATE %d != %d\n", m_currentState, data);

	for (byte x = 0; x < MAX_TURNOUTS; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			ModuleData moduleData;
			moduleData.setByteA(data);
			device->process(moduleData);
			data = moduleData.getByteA();
//			DEBUG_PRINT("TURNOUTDEVICE::process: DATA: %d\n", data);
		}
	}
	if (m_currentState != data)
	{
		m_currentState = data;
	}
	m_noWireHandler.writeNoWire(m_currentState);
}

void TurnoutModule::processWire(void)
{
	byte data(m_currentState);
	data = m_expander.read();
	for (byte x = 0; x < MAX_TURNOUTS; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			ModuleData moduleData;
			moduleData.setByteA(data);
			device->process(moduleData);
			data = moduleData.getByteA();
		}
	}

	if (m_currentState != data)
	{
//		DEBUG_PRINT("process:  CURRENT_STATE %d != %d\n", m_currentState, data);
		m_currentState = data;
		m_expander.write(m_currentState);
	}
}

void TurnoutModule::processUDPMessageWire(const UDPMessage &message)
{
//	DEBUG_PRINT("TurnoutModule::processUDPMessageWire: address: %d  messageID %d\n", getAddress(), message.getMessageID());
	byte data;
	data = m_expander.read();

	processUDPMessage(data, message);
	if (m_currentState != data)
	{
		//		DEBUG_PRINT("processUDPMessage:  CURRENT_STATE %d != %d\n", m_currentState, data);
		m_currentState = data;
		m_expander.write(m_currentState);
	}
}

void TurnoutModule::processUDPMessageNoWire(const UDPMessage &message)
{
	byte data(m_currentState);
	m_noWireHandler.readNoWire(data);

	processUDPMessage(data, message);
	if (m_currentState != data)
	{
		//		DEBUG_PRINT("processUDPMessage:  CURRENT_STATE %d != %d\n", m_currentState, data);
		m_currentState = data;
		m_noWireHandler.writeNoWire(data);
	}
}

void TurnoutModule::processUDPMessage(byte &data, const UDPMessage &message)
{
	for (byte x = 0; x < MAX_TURNOUTS; x++)
	{
		Device *device = getDevice(x);
		if (device)
		{
			ModuleData moduleData;
			moduleData.setByteA(data);
			device->processUDPMessage(moduleData, message);
			data = moduleData.getByteA();
		}
	}
}
