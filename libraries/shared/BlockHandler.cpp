#include <Arduino.h>
#include "BlockHandler.h"

BlockHandler::BlockHandler(void)
        : m_blockPin(0), m_currentState(0), m_current(LOW), m_lastRead(LOW), m_currentTimeout(0)
{
	memset(&m_config, 0, sizeof(BlockConfigStruct));
}

void BlockHandler::setup(byte blockPin)
{
	m_blockPin = blockPin;
}

bool BlockHandler::process(byte &data)
{
	bool ret = false;
	byte raw = bitRead(data, m_blockPin);
	long t = millis();
//	DEBUG_PRINT("%d\n", raw); 

	if (raw == m_lastRead)
	{
        if (raw != m_current && (t - m_currentTimeout) > 250 )
		{
			m_currentTimeout = t;
			m_current = raw;

			if (m_current == LOW)
			{
				m_currentState = 1;
				DEBUG_PRINT("Occupied TRUE deviceID: %d\n", m_config.deviceID);
				DEBUG_PRINT("-------------------------------\n%d\n", raw);
				DEBUG_PRINT("-------------------------------\n");
			}
			else
			{
				DEBUG_PRINT("Occupied FALSE %d\n", raw);
				m_currentState = 0;
			}
			ret = true;
		}
	}
	else
	{
		m_currentTimeout = t;
	}

	m_lastRead = raw;
	return ret;
}

bool BlockHandler::handleMessage(const Message &message)
{
	bool ret = false;
	
	return ret;
}

void BlockHandler::setConfigValue(const char *key, const char *value)
{
	if (strcmp(key, "ID") == 0)
	{
		int id = atoi(value);
		m_config.deviceID = id;
	}
}
