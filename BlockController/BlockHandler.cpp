#include <Arduino.h>
#include "BlockHandler.h"

BlockHandler::BlockHandler(void)
	: m_blockPin(0), m_blockID(-1), m_currentState(Unknown), m_current(HIGH), m_lastRead(HIGH), m_currentTimeout(0)
{
}

void BlockHandler::setup(byte blockPin)
{
	m_blockPin = blockPin;

	pinMode(m_blockPin, INPUT);
}

bool BlockHandler::process(void)
{
	bool ret = false;
	byte raw = digitalRead(m_blockPin);
	long t = millis();
//	DEBUG_PRINT("%d\n", raw); 

	if (raw == m_lastRead && raw != m_current && (t - m_currentTimeout) > 2000)
	{
		m_currentTimeout = t;
		m_current = raw;

		if (m_current == LOW)
		{
			m_currentState = Occupied;
			DEBUG_PRINT("Occupied TRUE\n");
			DEBUG_PRINT("-------------------------------\n%d\n", raw);
			DEBUG_PRINT("-------------------------------");
		}
		else
		{
			DEBUG_PRINT("Occupied FALSE %d\n", raw);
			m_currentState = Empty;
		}
		ret = true;
	}
	m_lastRead = raw;
	return ret;
}

bool BlockHandler::handleMessage(const Message &message)
{
	bool ret = false;
	
	return ret;
}
