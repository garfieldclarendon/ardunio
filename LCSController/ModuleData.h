#pragma once

#include <Arduino.h>

class ModuleData
{
public:
	ModuleData(void) : m_byteA(0), m_byteB(0) { }
	ModuleData(byte byteA, byte byteB) : m_byteA(byteA), m_byteB(byteB) { }
	ModuleData(const ModuleData &other) { m_byteA = other.m_byteA; m_byteB = other.m_byteB; }

	ModuleData& operator = (const ModuleData &other)
	{
		m_byteA = other.m_byteA;
		m_byteB = other.m_byteB;

		return *this;
	}

	byte getByteA(void) const { return m_byteA; }
	void setByteA(byte value) { m_byteA = value; }
	byte getByteB(void) const { return m_byteB; }
	void setByteB(byte value) { m_byteB = value; }

	void writeBit(byte bitIndex, byte data)
	{
		if (bitIndex < 8)
			bitWrite(m_byteA, bitIndex, data);
		else
			bitWrite(m_byteB, bitIndex - 8, data);
	}
	byte readBit(byte bitIndex)
	{
		if (bitIndex < 8)
			return bitRead(m_byteA, bitIndex);
		else
			return bitRead(m_byteB, bitIndex - 8);
	}
private:
	byte m_byteA;
	byte m_byteB;
};
