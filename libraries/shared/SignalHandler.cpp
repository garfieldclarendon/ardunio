#include <Arduino.h>
#include "SignalHandler.h"

#define blinkingTimeout 200

SignalHandler::SignalHandler(void) 
	: m_deviceID(0), m_redPin(0), m_yellowPin(0), m_greenPin(0), m_currentBlinkTimeout(0)
{ 
	memset(&m_blinkingPins, 255, 8);
}

bool SignalHandler::updateSignal(void)
{
  bool aspectChanged = false;
  return aspectChanged;
}

bool SignalHandler::process(void)
{
	bool ret = false;

	ret = updateSignal();

	blinkPins();

	return ret;
}

bool SignalHandler::handleMessage(const Message &)
{
	bool ret = false;

	return ret;
}

void SignalHandler::setup(byte redPin, byte yellowPin, byte greenPin)
{
	pinMode(m_redPin, OUTPUT);
	pinMode(m_yellowPin, OUTPUT);
	pinMode(m_greenPin, OUTPUT);
}

void SignalHandler::addBlinkingPin(byte pin)
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

void SignalHandler::removeBlinkingPin(byte pin)
{
	for (byte x = 0; x < 8; x++)
	{
		if (m_blinkingPins[x] == pin)
		{
			m_blinkingPins[x] = 255;
		}
	}
}

void SignalHandler::blinkPins(void)
{
	unsigned long t = millis();
	if (t - m_currentBlinkTimeout > blinkingTimeout)
	{
		m_currentBlinkTimeout = t;
		for (int x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] != 255)
			{
				bool value = (0 != (*portOutputRegister(digitalPinToPort(m_blinkingPins[x])) & digitalPinToBitMask(m_blinkingPins[x])));
				digitalWrite(m_blinkingPins[x], value == false);
			}
		}
	}
}