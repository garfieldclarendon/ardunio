#pragma once

#include "Message.h"
#include "GlobalDefs.h"

class SemaphoreHandler
{
public:
	SemaphoreHandler(void);
	void setup(byte motorAPin, byte motorBPin, byte normalPin, byte divergePin);
	bool process(void);
	void setSignal(byte motorPinSetting);

private:
	byte m_motorAPin;
	byte m_motorBPin;
	byte m_normalPin;
	byte m_divergePin;
	byte m_currentMotorSetting;

};

