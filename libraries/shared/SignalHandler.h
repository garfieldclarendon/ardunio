#pragma once

#include "ConfigStructures.h"
#include "Message.h"

#define MAX_SIGNAL_ASPECTS 10

class SignalHandler
{
public:
	SignalHandler(void);

	int getDeviceID(void) const { return m_deviceID; }
	void setDeviceID(int value) { m_deviceID = value; }
  
	void setup(byte redPin, byte yellowPin, byte greenPin);
	bool process(void);
	bool handleMessage(const Message &message);

	bool updateSignal(void);

private:
	void addBlinkingPin(byte pin);
	void removeBlinkingPin(byte pin);
	void blinkPins(void);
	int m_deviceID;
	byte m_redPin;
	byte m_yellowPin;
	byte m_greenPin;
	unsigned long m_currentBlinkTimeout;
	byte m_blinkingPins[8];
};

