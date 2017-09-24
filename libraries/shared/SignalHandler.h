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
  
	void setup(byte pin1, byte pin2, byte pin3);
	void setupWire(byte pin1, byte pin2, byte pin3);
	bool process(byte &data);
	void setSignal(PinStateEnum pin1State, PinStateEnum pin2State, PinStateEnum pin3State, byte &data);
	void setSignal(byte pin, PinStateEnum pinState, byte &data);

private:
	void addBlinkingPin(byte pin);
	void removeBlinkingPin(byte pin);
	void blinkPins(byte &data);
	int m_deviceID;
	byte m_pin1;
	byte m_pin2;
	byte m_pin3;
	unsigned long m_currentBlinkTimeout;
	byte m_blinkingPins[8];
};

