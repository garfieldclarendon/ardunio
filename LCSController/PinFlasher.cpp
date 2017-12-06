#include <Arduino.h>

#include "PinFlasher.h"

PinFlasher::PinFlasher(int timeout)
	: m_currentBlinkTimeout(0), m_blinkingTimeout(timeout)
{
	for (byte x = 0; x < 8; x++)
		m_blinkingPins[x] = 255;
}


PinFlasher::~PinFlasher()
{
}

void PinFlasher::addBlinkingPin(byte pin)
{
	bool found = false;
	// Make sure the pin is not already in the list
	for (byte x = 0; x < 8; x++)
	{
		if (m_blinkingPins[x] == pin)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		DEBUG_PRINT("Adding to blinking pins: %d\n", pin);
		// Add the pin to the first empty slot
		for (byte x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] == 255)
			{
				m_blinkingPins[x] = pin;
				break;
			}
		}
	}
}

void PinFlasher::removeBlinkingPin(byte pin)
{
	for (byte x = 0; x < 8; x++)
	{
		if (m_blinkingPins[x] == pin)
		{
			DEBUG_PRINT("Removing from blinking pins: %d\n", pin);
			m_blinkingPins[x] = 255;
		}
	}
}

void PinFlasher::blinkPins(byte &data)
{
	unsigned long t = millis();
	static bool flash = true;
	if (t - m_currentBlinkTimeout > m_blinkingTimeout)
	{
		m_currentBlinkTimeout = t;
		for (byte x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] != 255)
			{
				bitWrite(data, m_blinkingPins[x], flash);
//				DEBUG_PRINT("BLINKING PIN %d\n", m_blinkingPins[x]);
			}
		}
		flash = !flash;
	}
}
