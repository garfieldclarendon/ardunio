#pragma once

#include "GlobalDefs.h"

class PinFlasher
{
public:
	PinFlasher(int timout);
	~PinFlasher();

	void addBlinkingPin(byte pin);
	void blinkPins(byte &data);
	void removeBlinkingPin(byte pin);

private:
	unsigned long m_currentBlinkTimeout;
	byte m_blinkingPins[8];
	int m_blinkingTimeout;
};

