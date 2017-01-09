#pragma once

#include "SignalAspectCondition.h"
#include "ConfigStructures.h"
#include "Message.h"

#define MAX_SIGNAL_ASPECTS 10

class SignalHandler
{
public:
	SignalHandler(void);

	int getDeviceID(void) const { return m_deviceID; }
	void setDeviceID(int value) { m_deviceID = value; }
	SignalAspect getCurrentAspect(void) const { return m_currentAspect; }
  
	void setup(byte redPin, byte yellowPin, byte greenPin);
	bool process(void);
	bool handleMessage(const Message &message);

	bool updateSignal(void);
  
	void setConfig(const SignalConfigStruct &config);
	void setSignal(SignalAspect newState);

private:
	void addBlinkingPin(byte pin);
	void removeBlinkingPin(byte pin);
	void blinkPins(void);
	int m_deviceID;
	SignalAspectCondition m_conditions[MAX_SIGNAL_CONDITIONS];
	SignalAspect m_currentAspect;
	byte m_redPin;
	byte m_yellowPin;
	byte m_greenPin;
	unsigned long m_currentBlinkTimeout;
	byte m_blinkingPins[8];
};

