#include "NoWireHandler.h"
#include "Device.h"
#include "ModuleData.h"

NoWireHandler::NoWireHandler()
	: m_ioDir(0)
{
	memset(m_noWirePinMap, 0, 8);
}

void NoWireHandler::setNoWirePinDir(byte ioDir)
{
	m_ioDir = ioDir;
	for (byte x = 0; x < 8; x++)
	{
		if (bitRead(ioDir, x) == LOW)
		{
			DEBUG_PRINT("SET PIN '%d' OUTPUT\n", m_noWirePinMap[x]);
			pinMode(m_noWirePinMap[x], OUTPUT);
		}
		else
		{
			DEBUG_PRINT("SET PIN '%d' INPUT\n", m_noWirePinMap[x]);
			pinMode(m_noWirePinMap[x], INPUT);
		}
	}
}

void NoWireHandler::readNoWire(byte &data)
{
	for (byte x = 0; x < 8; x++)
	{
		if (bitRead(m_ioDir, x) == HIGH)
		{
//			DEBUG_PRINT("READ PIN '%d' BIT: '%d' VALUE: %d\n", m_noWirePinMap[x], x, digitalRead(m_noWirePinMap[x]));
			bitWrite(data, x, digitalRead(m_noWirePinMap[x]));
		}
	}
}

void NoWireHandler::writeNoWire(byte data)
{
	for (byte x = 0; x < 8; x++)
	{
		if (bitRead(m_ioDir, x) == LOW)
		{
//			DEBUG_PRINT("WRITE PIN '%d' VALUE: %d\n", m_noWirePinMap[x], bitRead(data, x));
			digitalWrite(m_noWirePinMap[x], bitRead(data, x));
		}
	}
}
