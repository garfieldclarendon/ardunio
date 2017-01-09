#include <Arduino.h>
#include "SemaphoreHandler.h"

SemaphoreHandler::SemaphoreHandler(void)
	: m_motorAPin(0), m_motorBPin(0), m_normalPin(0), m_divergePin(0)
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

	SignalAspect aspect;
	aspect.setRedMode(SignalAspect::On);
	setSignal(aspect);
}

bool SemaphoreHandler::process(void)
{
	bool ret = false;

	ret = m_signalHandler.process();

	if (ret)
		setSignal(m_signalHandler.getCurrentAspect());

	return ret;
}

bool SemaphoreHandler::handleMessage(const Message &message)
{
	bool ret = false;
	
	ret = m_signalHandler.handleMessage(message);

	return ret;
}

Message SemaphoreHandler::createMessage(SignalAspect newState)
{
	Message message;

	message.setDeviceID(m_signalHandler.getDeviceID());
	message.setMessageID(TRN_STATUS);
	message.setMessageClass(ClassSignal);
	message.setIntValue1(newState.getRedMode());
	message.setByteValue1(newState.getYellowMode());
	message.setByteValue2(newState.getGreenMode());

	return message;
}

void SemaphoreHandler::setSignal(SignalAspect newState)
{
	if (newState.getRedMode() == SignalAspect::On || newState.getRedMode() == SignalAspect::Flashing)
	{
		Serial.println("SETTING SIGNAL TO RED");
		digitalWrite(m_motorAPin, 1);
		digitalWrite(m_motorBPin, 0);
	}
	else if (newState.getGreenMode() == SignalAspect::On || newState.getGreenMode() == SignalAspect::Flashing)
	{
		Serial.println("SETTING SIGNAL TO GREEN");
		digitalWrite(m_motorAPin, 0);
		digitalWrite(m_motorBPin, 1);
	}
}
