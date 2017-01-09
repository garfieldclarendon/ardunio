#pragma once

#include "Message.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"
#include "SignalAspectCondition.h"
#include "SignalHandler.h"
#include "SignalAspect.h"

class SemaphoreHandler
{
public:
	SemaphoreHandler(void);
	void setup(byte motorAPin, byte motorBPin, byte normalPin, byte divergePin);
	void setConfig(SignalConfigStruct value) { m_signalHandler.setConfig(value); }
	bool process(void);
	bool handleMessage(const Message &message);
	SignalAspect getCurrentAspect(void) const { return m_signalHandler.getCurrentAspect(); }
	void setSignal(SignalAspect newState);

	int getSignalID(void) const { return m_signalHandler.getDeviceID(); }
	Message createMessage(SignalAspect newState);

private:
	SignalHandler m_signalHandler;
	byte m_motorAPin;
	byte m_motorBPin;
	byte m_normalPin;
	byte m_divergePin;
};

