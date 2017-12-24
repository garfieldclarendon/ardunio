#pragma once

#include <Arduino.h>

class ModuleData
{
public:
	ModuleData(void) : m_byteA(0), m_byteB(0), m_flashFlagA(0), m_flashFlagB(0) { }
	ModuleData(byte byteA, byte byteB) : m_byteA(byteA), m_byteB(byteB), m_flashFlagA(0), m_flashFlagB(0) { }
	ModuleData(const ModuleData &other) 
	{ 
		m_byteA = other.m_byteA; 
		m_byteB = other.m_byteB; 
		m_flashFlagA = other.m_flashFlagA;
		m_flashFlagB = other.m_flashFlagB;
	}

	ModuleData& operator = (const ModuleData &other)
	{
		m_byteA = other.m_byteA;
		m_byteB = other.m_byteB;
		m_flashFlagA = other.m_flashFlagA;
		m_flashFlagB = other.m_flashFlagB;

		return *this;
	}

	byte getByteA(void) const { return m_byteA; }
	void setByteA(byte value) { m_byteA = value; }
	byte getByteB(void) const { return m_byteB; }
	void setByteB(byte value) { m_byteB = value; }

	void setFlashOn(byte bitIndex)
	{
		if (bitIndex < 8)
			bitWrite(m_flashFlagA, bitIndex, HIGH);
		else
			bitWrite(m_flashFlagB, bitIndex - 8, HIGH);
	}

	void setFlashOff(byte bitIndex)
	{
		if (bitIndex < 8)
			bitWrite(m_flashFlagA, bitIndex, LOW);
		else
			bitWrite(m_flashFlagB, bitIndex - 8, LOW);
	}

	bool isFlashSet(byte bitIndex) const
	{
		if (bitIndex < 8)
			return bitRead(m_flashFlagA, bitIndex) == HIGH;
		else
			return bitRead(m_flashFlagB, bitIndex - 8) == HIGH;
	}

	void writeBit(byte bitIndex, byte data)
	{
		setFlashOff(bitIndex);
		if (bitIndex < 8)
			bitWrite(m_byteA, bitIndex, data);
		else
			bitWrite(m_byteB, bitIndex - 8, data);
	}

	byte readBit(byte bitIndex) const
	{
		if (bitIndex < 8)
			return bitRead(m_byteA, bitIndex);
		else
			return bitRead(m_byteB, bitIndex - 8);
	}
private:
	byte m_byteA;
	byte m_byteB;
	byte m_flashFlagA;
	byte m_flashFlagB;
};
