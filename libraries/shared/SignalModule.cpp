#include <Arduino.h>
#include "SignalModule.h"

SignalModule::SignalModule(void)
	: m_currentState(0)
{
}

byte SignalModule::getIODirConfig(void) const
{
	// Set all pins to output
	byte iodir = 0;

	return iodir;
}

void SignalModule::setup(void)
{
	byte bit = 0;
	for (byte x = 0; x < MAX_SIGNALS; x++)
	{
		setup(x, bit, bit + 1, bit + 2);
		bit += 3;
	}

	m_currentState = 0;
}

void SignalModule::setupWire(byte address)
{
	DEBUG_PRINT("SignalModule::setupWire:  address %d\n", address);
	setAddress(address);
	byte iodir = getIODirConfig();

	byte bit = 0;
	for (byte x = 0; x < MAX_SIGNALS; x++)
	{
		m_signals[x].setupWire(bit, bit + 1, bit + 2);
		bit += 3;
	}

	expanderWrite(IODIR, iodir);
	delay(100);
	// Turn on all lights
	byte data = 0;
	expanderWrite(GPIO, data);
	DEBUG_PRINT("SignalModule::setupWire:  address %d DONE!!!!\n", address);
}

void SignalModule::setup(byte index, byte pin1, byte pin2, byte pin3)
{
	DEBUG_PRINT("SignalModule::setup:  index %d\n", index);
	m_signals[index].setup(pin1, pin2, pin3);
}

bool SignalModule::process(byte &data)
{
	if (Network.getIsConnected())
	{
		for (byte x = 0; x < MAX_SIGNALS; x++)
			m_signals[x].process(data);
	}
	else
	{
		data = 0;
	}
//	DEBUG_PRINT("process:  CURRENT_STATE %d === %d\n  STATUS_1 %d  STATUS_2 %d\n", m_currentState, data, m_signals[0].getCurrentState(), m_signals[1].getCurrentState());

	if (m_currentState != data)
	{
//		DEBUG_PRINT("process:  CURRENT_STATE %d != %d\n\n", m_currentState, data);
		m_currentState = data;
	}

	bool ret = false;
	return ret;
}

void SignalModule::netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject & json, byte &data)
{
	if (action == NetActionUpdate)
	{
		int port = json["port"];
		PinStateEnum pin1State = ((PinStateEnum)(int)json["pin1State"]);
		PinStateEnum pin2State = ((PinStateEnum)(int)json["pin2State"]);
		PinStateEnum pin3State = ((PinStateEnum)(int)json["pin3State"]);

		setSignal(port, pin1State, pin2State, pin3State);
		DEBUG_PRINT("netModuleCallback:  Setting Signal %d -- %d\n\n", m_currentState, data);
		data = m_currentState;
	}
}

void SignalModule::netModuleConfigCallback(NetActionType /* action */, byte /* moduleIndex */, const JsonObject & /* json */)
{
}

void SignalModule::setSignal(byte port, PinStateEnum pin1State, PinStateEnum pin2State, PinStateEnum pin3State)
{
	m_signals[port].setSignal(pin1State, pin2State, pin3State, m_currentState);
}

void SignalModule::sendStatusMessage(void)
{
	StaticJsonBuffer<500> jsonBuffer;
	JsonObject &out = jsonBuffer.createObject();
	out["messageUri"] = "/controller/module";
	out["moduleIndex"] = getAddress();
	out["class"] = (int)ClassSignal;
	out["action"] = (int)NetActionGet;
	Network.sendMessageToServer(out);
}