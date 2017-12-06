#pragma once

#include <Arduino.h>

class Expander16Bit
{
public:
	Expander16Bit(void);
	Expander16Bit(byte address);

	void expanderBegin(byte ioDirA, byte ioDirB);
	byte readA(void);
	byte readB(void);
	void writeA(const byte data);
	void writeB(const byte data);

	byte getAddress(void) const { return m_address; }
	void setAddress(byte value) { m_address = value; }

private:
	void expanderWrite(const byte reg, const byte data);
	void expanderWriteBoth(const byte reg, const byte data);
	byte expanderRead(const byte reg);
	int I2C_ClearBus(void);
	byte m_address;
};

