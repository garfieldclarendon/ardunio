#include <Arduino.h>
#include "SemaphoreHandler.h"

SemaphoreHandler::SemaphoreHandler(void)
	: m_motorAPin(0), m_motorBPin(0), m_normalPin(0), m_divergePin(0), m_currentMotorSetting(0)
{
}

void SemaphoreHandler::setup(byte motorAPin, byte motorBPin, byte normalPin, byte divergePin)
{
	m_motorAPin = motorAPin;
	m_motorBPin = motorBPin;
	m_normalPin = normalPin;
	m_divergePin = divergePin;

	pinMode(m_motorAPin, OUTPUT);
	pinMode(m_motorBPin, OUTPUT);
	pinMode(m_normalPin, INPUT);
	pinMode(m_divergePin, INPUT);

	digitalWrite(m_motorAPin, 0);
	digitalWrite(m_motorBPin, 1);
}

bool SemaphoreHandler::process(void)
{
	bool ret = true;

	if (m_currentMotorSetting == 0)
	{
		digitalWrite(m_motorAPin, 0);
		digitalWrite(m_motorBPin, 1);
	}
	else
	{
		digitalWrite(m_motorAPin, 1);
		digitalWrite(m_motorBPin, 0);
	}

	return ret;
}

void SemaphoreHandler::setSignal(byte motorPinSetting)
{
	m_currentMotorSetting = motorPinSetting;
}
