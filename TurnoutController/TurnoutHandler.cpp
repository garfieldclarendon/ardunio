#include <Arduino.h>
#include "TurnoutHandler.h"

TurnoutHandler::TurnoutHandler(void)
	: m_motorAPin(0), m_motorBPin(0), m_normalPin(0), m_divergePin(0), m_currentState(TrnUnknown), m_lastState(TrnUnknown), m_currentTimeout(0)
{
	memset(&m_config, 0, sizeof(TurnoutConfigStruct));
}

void TurnoutHandler::setup(byte motorAPin, byte motorBPin, byte normalPin, byte divergePin)
{
	m_motorAPin = motorAPin;
	m_motorBPin = motorBPin;
	m_normalPin = normalPin;
	m_divergePin = divergePin;

	pinMode(m_motorAPin, OUTPUT);
	pinMode(m_motorBPin, OUTPUT);
	pinMode(m_normalPin, INPUT);
	pinMode(m_divergePin, INPUT);
}

bool TurnoutHandler::process(void)
{
	bool ret = false;
	TurnoutState current = readCurrentState();
	long t = millis();
	if (current != TrnUnknown && (t - m_currentTimeout) > TIMEOUT_INTERVAL)
	{
		if (current == m_lastState && m_lastState != m_currentState)
		{
			if (m_currentState == TrnToDiverging && current == TrnNormal)
			{
				
			}
			else if (m_currentState == TrnToNormal && current == TrnDiverging)
			{

			}
			else
			{
				m_currentState = current;
				ret = true;
			}
		}
	}

	m_lastState = current;

	return ret;
}

bool TurnoutHandler::handleMessage(const Message &message)
{
	bool ret = false;

	TurnoutState newState(TrnUnknown);
	if (message.getMessageID() == TRN_ACTIVATE)
	{
		if (message.getIntValue1() == m_config.turnoutID)
			newState = (TurnoutState)message.getByteValue1();
		if(message.getIntValue2() == m_config.turnoutID)
			newState = (TurnoutState)message.getByteValue2();
		Serial.print("handleMessage:  setTurnout for deviceID: ");
		Serial.println(m_config.turnoutID);
		Serial.print("New State:  ");
		Serial.println(newState);
	}
	else if (message.getMessageID() == PANEL_ACTIVATE_ROUTE)
	{
		Serial.print("handleMessage:  setTurnout for routeID: ");
		Serial.println(message.getDeviceID());
		newState = getTurnoutStateForRoute(message.getDeviceID());
	}
	if (newState != TrnUnknown)
	{
		setTurnout(newState);
		if (newState == TrnNormal)
			m_currentState = TrnToNormal;
		else
			m_currentState = TrnToDiverging;
		ret = true;
	}
	
	return ret;
}

TurnoutState TurnoutHandler::getTurnoutStateForRoute(int routeID)
{
	TurnoutState state(TrnUnknown);

	for (byte x = 0; x < MAX_ROUTE_ENTRIES; x++)
	{
		if (m_config.routeEntries[x].routeID == routeID)
		{
			state = m_config.routeEntries[x].state;
			break;
		}
	}
	return state;
}

TurnoutState TurnoutHandler::readCurrentState(void)
{
	TurnoutState current(TrnUnknown);

	if (digitalRead(m_normalPin) == LOW)
	{
		current = TrnNormal;
	}
	else if (digitalRead(m_divergePin) == LOW)
	{
		current = TrnDiverging;
	}

	return current;
}

Message TurnoutHandler::createMessage(TurnoutState newState)
{
	Message message;

	message.setDeviceID(m_config.turnoutID);
	message.setMessageID(TRN_STATUS);
	message.setMessageClass(ClassTurnout);
	message.setByteValue1(newState);

	return message;
}

void TurnoutHandler::setTurnout(TurnoutState newState)
{
	if (newState == TrnDiverging || newState == TrnToDiverging)
	{
		Serial.println("SETTING TURNOUT TO DIVERGING");
		digitalWrite(m_motorAPin, 1);
		digitalWrite(m_motorBPin, 0);
	}
	else if (newState == TrnNormal || newState == TrnToNormal)
	{
		Serial.println("SETTING TURNOUT TO NORMAL");
		digitalWrite(m_motorAPin, 0);
		digitalWrite(m_motorBPin, 1);
	}
}
