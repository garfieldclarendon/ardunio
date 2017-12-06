#pragma once

#include <Arduino.h>

class NoWireHandler 
{
public:
	NoWireHandler();

	void setNoWirePinMap(byte pinMap[8]) { memcpy(m_noWirePinMap, pinMap, 8); }
	void setNoWirePinDir(byte ioDir);
	void readNoWire(byte &data);
	void writeNoWire(byte data);

private:
	byte m_noWirePinMap[8];
	byte m_ioDir;
};
