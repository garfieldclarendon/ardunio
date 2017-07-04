#include <Arduino.h>
#include "SignalHandler.h"

#define blinkingTimeout 750

SignalHandler::SignalHandler(void) 
	: m_deviceID(0), m_pin1(0), m_pin2(0), m_pin3(0), m_currentBlinkTimeout(0)
{ 
	memset(&m_blinkingPins, 255, 8);
}

bool SignalHandler::process(byte &data)
{
	bool ret = false;

	blinkPins(data);

	return ret;
}

void SignalHandler::setSignal(PinStateEnum pin1State, PinStateEnum pin2State, PinStateEnum pin3State, byte &data)
{
	setSignal(m_pin1, pin1State, data);
	setSignal(m_pin2, pin2State, data);
	setSignal(m_pin3, pin3State, data);
}

void SignalHandler::setSignal(byte pin, PinStateEnum pinState, byte &data)
{
	if (pinState == PinFlashing)
	{
		addBlinkingPin(pin);
	}
	else
	{
		removeBlinkingPin(pin);
		if (pin < 8)
			bitWrite(data, pin, PinOn == pinState ? 1 : 0);
	}
}

void SignalHandler::setup(byte pin1, byte pin2, byte pin3)
{
	m_pin1 = pin1;
	m_pin2 = pin2;
	m_pin3 = pin3;

	if (m_pin1 < 8)
		pinMode(m_pin1, OUTPUT);
	if (m_pin2 < 8)
		pinMode(m_pin2, OUTPUT);
	if (m_pin3 < 8)
		pinMode(m_pin3, OUTPUT);
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

void SignalHandler::blinkPins(byte &data)
{
	unsigned long t = millis();
	if (t - m_currentBlinkTimeout > blinkingTimeout)
	{
		m_currentBlinkTimeout = t;
		for (int x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] != 255)
			{
				int state = bitRead(data, m_blinkingPins[x]);
				bitWrite(data, m_blinkingPins[x], state == 0);
			}
		}
	}
}