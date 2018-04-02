#pragma once

#include <Arduino.h>

class Expander8Bit
{
public:
	Expander8Bit();

	void expanderBegin(byte ioDir);
	void write(const byte data);
	byte read(void);
	byte getAddress(void) const { return m_address; }
	void setAddress(byte value) { m_address = value; }

private:
	int I2C_ClearBus(void);
	void expanderWrite(const byte reg, const byte data);
	byte expanderRead(const byte reg);
	byte m_address;
	byte m_retryCount;
};

