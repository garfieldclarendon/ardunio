#include <Arduino.h>
#include "TurnoutHandler.h"

TurnoutHandler::TurnoutHandler(void)
	: m_motorAPin(0), m_motorBPin(0), m_normalPin(0), m_divergePin(0), m_currentState(TrnUnknown), m_lastState(TrnUnknown), m_currentTimeout(0), m_currentRouteConfig(0)
{
	m_config = NULL;
}

void TurnoutHandler::setup(byte motorAPin, byte motorBPin, byte normalPin, byte divergePin)
{
	m_motorAPin = motorAPin;
	m_motorBPin = motorBPin;
	m_normalPin = normalPin;
	m_divergePin = divergePin;
}

bool TurnoutHandler::process(byte &data)
{
	bool ret = false;
	TurnoutState current = readCurrentState(data);
	long t = millis();
	if (current != TrnUnknown && (t - m_currentTimeout) > TIMEOUT_INTERVAL)
	{
		if (current == m_lastState && m_lastState != m_currentState)
		{
			if (m_currentState == TrnToDiverging && current == TrnNormal)
			{
//				DEBUG_PRINT("%d TurnoutHandler::process1:  CURRENT_STATE %d != %d CURRENT\n", getTurnoutID(), m_currentState, current);
			}
			else if (m_currentState == TrnToNormal && current == TrnDiverging)
			{
//				DEBUG_PRINT("%d TurnoutHandler::process2:  CURRENT_STATE %d != %d CURRENT\n", getTurnoutID(), m_currentState, current);
			}
			else
			{
				DEBUG_PRINT("%d TurnoutHandler::process3:  CURRENT_STATE %d != %d CURRENT SETTING CURRENT STATE\n", getTurnoutID(), m_currentState, current);
				m_currentState = current;
				ret = true;
			}
		}
	}
	else
	{
//		DEBUG_PRINT("%d TurnoutHandler::process4:  CURRENT_STATE %d != %d CURRENT IS UNKNOWN!!  DATA: %d\n", getTurnoutID(), m_currentState, current, data);
	}

	m_lastState = current;

	if (m_currentState == TrnNormal || m_currentState == TrnToNormal || m_currentState == TrnUnknown)
	{
		bitWrite(data, m_motorAPin, 0);
		bitWrite(data, m_motorBPin, 1);
	}
	else
	{
		bitWrite(data, m_motorAPin, 1);
		bitWrite(data, m_motorBPin, 0);
	}

	return ret;
}

TurnoutState TurnoutHandler::getTurnoutStateForRoute(int routeID)
{
	TurnoutState state(TrnUnknown);

	for (byte x = 0; x < MAX_ROUTE_ENTRIES; x++)
	{
		if (m_config->routeEntries[x].routeID == routeID)
		{
			state = m_config->routeEntries[x].state;
			break;
		}
	}
	return state;
}

TurnoutState TurnoutHandler::readCurrentState(byte data)
{
	TurnoutState current(TrnUnknown);

	if (bitRead(data, m_normalPin) == LOW)
		current = TrnNormal;
	else if (bitRead(data, m_divergePin) == LOW)
		current = TrnDiverging;

	return current;
}

void TurnoutHandler::setTurnout(TurnoutState newState, byte &data)
{
	if (newState != TrnUnknown && newState != TrnToDiverging && newState != TrnToNormal)
	{
		if (newState == TrnDiverging && m_currentState != newState)
		{
			DEBUG_PRINT("SETTING TURNOUT TO DIVERGING\n");
			bitWrite(data, m_motorAPin, 1);
			bitWrite(data, m_motorBPin, 0);
			if (bitRead(data, m_divergePin) != LOW || m_currentState == TrnUnknown)
				m_currentState = TrnToDiverging;
		}
		else if (newState == TrnNormal && m_currentState != newState)
		{
			DEBUG_PRINT("SETTING TURNOUT TO NORMAL\n");
			bitWrite(data, m_motorAPin, 0);
			bitWrite(data, m_motorBPin, 1);
			if (bitRead(data, m_normalPin) != LOW || m_currentState == TrnUnknown)
				m_currentState = TrnToNormal;
		}
	}
	DEBUG_PRINT("SETTURNOUT: %d\n", data);
}

void TurnoutHandler::setConfigValue(const char *key, const char *value)
{
	if (strcmp(key, "ID") == 0)
	{
		int id = atoi(value);
		m_config->turnoutID = id;
	}
	else if (strcmp(key, "ROUTE") == 0)
	{
		int id = atoi(value);
		m_config->routeEntries[m_currentRouteConfig].routeID = id;

	}
	else if (strcmp(key, "STATE") == 0)
	{
		int state = atoi(value);
		m_config->routeEntries[m_currentRouteConfig++].state = (TurnoutState)state;
	}
}

void TurnoutHandler::setConfig(TurnoutConfigStruct *value)
{ 
	m_config = value;
}
