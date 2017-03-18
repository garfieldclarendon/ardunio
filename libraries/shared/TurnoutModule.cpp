#include <Arduino.h>
#include "TurnoutModule.h"

TurnoutModule::TurnoutModule(void)
	: m_currentState(0), m_currentTurnoutConfig(-1)

{
	memset(&m_config, 0, sizeof(TurnoutControllerConfigStruct));
	m_turnouts[0].setConfig(&m_config.turnout1);
	m_turnouts[1].setConfig(&m_config.turnout2);
}

byte TurnoutModule::setupWire(byte address)
{
	m_address = address;
	byte iodir = 0;
	byte motorAPin1 = 0;
	byte motorBPin1 = 1;
	byte motorAPin2 = 4;
	byte motorBPin2 = 5;
	byte normalPin1 = 2;
	byte divergePin1 = 3;
	byte normalPin2 = 6;
	byte divergePin2 = 7;

	if (m_config.turnout1.inputPinSetting == 1)
	{
		byte hold = normalPin1;
		normalPin1 = divergePin1;
		divergePin1 = hold;
		hold = motorAPin1;
		motorAPin1 = motorBPin1;
		motorBPin1 = hold;
	}
	if (m_config.turnout2.inputPinSetting == 1)
	{
		byte hold = normalPin2;
		normalPin2 = divergePin2;
		divergePin2 = hold;
		hold = motorAPin2;
		motorAPin2 = motorBPin2;
		motorBPin2 = hold;
	}

	bitWrite(iodir, motorAPin1, 0);
	bitWrite(iodir, motorBPin1, 0);
	bitWrite(iodir, motorAPin2, 0);
	bitWrite(iodir, motorBPin2, 0);
	bitWrite(iodir, normalPin1, 1);
	bitWrite(iodir, divergePin1, 1);
	bitWrite(iodir, normalPin2, 1);
	bitWrite(iodir, divergePin2, 1);

	setup(0, motorAPin1, motorBPin1, normalPin1, divergePin1);
	setup(1, motorAPin2, motorBPin2, normalPin2, divergePin2);

	m_currentState = 0;
	// Force the diverging pin ON so that it resets to normal
	// if the turnout happens to be set to the diverging route
	bitWrite(m_currentState, normalPin1, 1);
	bitWrite(m_currentState, divergePin1, 0);
	bitWrite(m_currentState, normalPin2, 1);
	bitWrite(m_currentState, divergePin2, 0);

	DEBUG_PRINT("TurnoutModule::setup  Turnout1\n");
	m_turnouts[0].setTurnout(TrnNormal, m_currentState);
	DEBUG_PRINT("TurnoutModule::setup  Turnout2\n");
	m_turnouts[1].setTurnout(TrnNormal, m_currentState);

	return iodir;
}

void TurnoutModule::setup(byte index, byte motorAPin, byte motorBPin, byte normalPin, byte divergePin)
{
	m_turnouts[index].setup(motorAPin, motorBPin, normalPin, divergePin);
}

bool TurnoutModule::process(byte &data)
{
	bool sendStatus = false;
	for (byte x = 0; x < MAX_TURNOUTS; x++)
		if (m_turnouts[x].process(data))
			sendStatus = true;
	
	if (m_currentState != data)
	{
		DEBUG_PRINT("process:  CURRENT_STATE %d != %d\n  STATUS_1 %d  STATUS_2 %d\n", m_currentState, data, m_turnouts[0].getCurrentState(), m_turnouts[1].getCurrentState());
		m_currentState = data;
	}

	bool ret = false;
	if (sendStatus)
	{
		ret = true;
	}

	return ret;
}

bool TurnoutModule::handleMessage(const Message &message, byte &data)
{
	bool ret = false;

	TurnoutState newTurnoutState(TrnUnknown);
	if (message.getMessageID() == TRN_ACTIVATE || message.getMessageID() == MULTI_STATUS)
	{
		for (byte x = 0; x < MAX_TURNOUTS; x++)
		{
			for (byte index = 0; index < MAX_MODULES; index++)
			{
				if (message.getDeviceStatusID(index) > 0)
				{
					newTurnoutState = (TurnoutState)message.getDeviceStatus(index);
					if (message.getDeviceStatusID(index) == getTurnoutID(x))
					{
						DEBUG_PRINT("handleMessage:  setTurnout for deviceID: %d\nNew State: %d\n", getTurnoutID(x), newTurnoutState);
						setTurnout(x, newTurnoutState, data);
						ret = true;
					}
				}
			}
		}
	}
	else if (message.getMessageID() == PANEL_ACTIVATE_ROUTE)
	{
		DEBUG_PRINT("handleMessage:  setTurnout for routeID: %d\n", message.getDeviceID());
		for (byte x = 0; x < MAX_TURNOUTS; x++)
		{
			newTurnoutState = m_turnouts[x].getTurnoutStateForRoute(message.getDeviceID());
			if (newTurnoutState != TrnUnknown)
			{
				setTurnout(x, newTurnoutState, data);
				ret = true;
			}
		}
	}

	if (m_currentState != data)
	{
		DEBUG_PRINT("handleMessage:  CURRENT_STATE %d != %d\n", m_currentState, data);
		m_currentState = data;
	}

	return ret;
}

TurnoutState TurnoutModule::getTurnoutStateForRoute(int routeID)
{
	TurnoutState state(TrnUnknown);

	for (byte x = 0; x < getDeviceCount(); x++)
	{
		state = m_turnouts[x].getTurnoutStateForRoute(routeID);
		if (state != TrnUnknown)
			break;
	}
	return state;
}

Message TurnoutModule::createMessage(void)
{
	Message message;

	message.setMessageID(TRN_STATUS);
	message.setMessageClass(ClassTurnout);
	message.setDeviceStatus(0, getTurnoutID(0), m_turnouts[0].getCurrentState());
	message.setDeviceStatus(1, getTurnoutID(1), m_turnouts[1].getCurrentState());

	return message;
}

void TurnoutModule::setTurnout(byte index, TurnoutState newTurnoutState, byte &data)
{
	m_turnouts[index].setTurnout(newTurnoutState, data);
}

void TurnoutModule::configCallback(const char *key, const char *value)
{
	if (strcmp(key, "ID") == 0)
	{
		m_currentTurnoutConfig++;
		if (m_currentTurnoutConfig == MAX_TURNOUTS)
			m_currentTurnoutConfig = 0; 
	}
	m_turnouts[m_currentTurnoutConfig].setConfigValue(key, value);
	m_config.turnout1 = m_turnouts[0].getConfig();
	m_config.turnout2 = m_turnouts[1].getConfig();
}

const char *TurnoutModule::getConfigReference(void) const
{

	return (const char *)&m_config;
}

int TurnoutModule::getConfigSize(void) const
{
	return sizeof(TurnoutControllerConfigStruct);
}
