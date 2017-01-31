#include <Arduino.h>
#include "TurnoutModule.h"

TurnoutModule::TurnoutModule(void)
	: m_currentState(0)
{
}

void TurnoutModule::setup(void)
{
	m_turnouts[0].setTurnout(TrnNormal, m_currentState);
	m_turnouts[1].setTurnout(TrnNormal, m_currentState);
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
	if (message.getMessageID() == TRN_ACTIVATE)
	{
		for (byte x = 0; x < MAX_TURNOUTS; x++)
		{
			newTurnoutState = (TurnoutState)message.getDeviceStatus(0);
			if (message.getDeviceStatusID(0) == getTurnoutID(x))
			{
				DEBUG_PRINT("handleMessage:  setTurnout for deviceID: %d\nNew State: %d\n", getTurnoutID(x), newTurnoutState);
				setTurnout(x, newTurnoutState, data);
				ret = true;
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

	for (byte x = 0; x < getTurnoutCount(); x++)
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
	message.setIntValue1(getTurnoutID(0));
	message.setIntValue2(getTurnoutID(1));
	message.setByteValue1(m_turnouts[0].getCurrentState());
	message.setByteValue2(m_turnouts[1].getCurrentState());

	return message;
}

void TurnoutModule::setTurnout(byte index, TurnoutState newTurnoutState, byte &data)
{
	m_turnouts[index].setTurnout(newTurnoutState, data);
}
