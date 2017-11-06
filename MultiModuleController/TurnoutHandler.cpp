#include <Arduino.h>
#include "TurnoutHandler.h"

TurnoutHandler::TurnoutHandler(void)
	: m_motorAPin(0), m_motorBPin(0), m_feedbackAPin(0), m_feedbackBPin(0), m_lastARead(0), m_lastBRead(0), m_currentATimeout(0), m_currentBTimeout(0), m_currentMotorSetting(0), m_currentFeedbackA(0), m_currentFeedbackB(0)
{
	m_config = NULL;
}

void TurnoutHandler::setup(byte motorAPin, byte motorBPin, byte feedbackAPin, byte feedbackBPin)
{
	DEBUG_PRINT("TurnoutHandler::setup:  motorAPin %d motorBAPin %d feedbackAPin %d feedbackBPin %d\n", motorAPin, motorBPin, feedbackAPin, feedbackBPin);

	m_motorAPin = motorAPin;
	m_motorBPin = motorBPin;
	m_feedbackAPin = feedbackAPin;
	m_feedbackBPin = feedbackBPin;
}

bool TurnoutHandler::process(byte &data)
{
	bool ret = false;

	if (m_currentMotorSetting == 0)
	{
		bitWrite(data, m_motorAPin, 0);
		bitWrite(data, m_motorBPin, 1);
	}
	else
	{
		bitWrite(data, m_motorAPin, 1);
		bitWrite(data, m_motorBPin, 0);
	}

	// Debounce the read of the input pins.  Make sure their current state has been stable for at least 100 milliseconds

	// Process the inputA pin
	long t = millis();
	byte current = bitRead(data, m_feedbackAPin);
	if (current == m_lastARead)
	{
		if (current != m_currentFeedbackA && (t - m_currentATimeout) > 250)
		{
			m_currentATimeout = t;
			m_currentFeedbackA = current;
	//		DEBUG_PRINT("FEEDBACK A PIN CHANGED: %d\n", m_currentFeedbackA);
			ret = true;
		}
	}
	else
	{
		m_currentATimeout = t;
	}
	m_lastARead = current;
	//
	// Process the inputB pin
	current = bitRead(data, m_feedbackBPin);
	if (current == m_lastBRead)
	{
		if (current != m_currentFeedbackB && (t - m_currentBTimeout) > 250)
		{
			m_currentBTimeout = t;
			m_currentFeedbackB = current;
	//		DEBUG_PRINT("FEEDBACK B PIN CHANGED: %d\n", m_currentFeedbackA);
			ret = true;
		}
	}
	else
	{
		m_currentBTimeout = t;
	}
	m_lastBRead = current;

	return ret;
}

void TurnoutHandler::setTurnout(byte motorPinSetting)
{
	DEBUG_PRINT("SETTURNOUT: %d CURRENT STATE: %d NEW STATE: %d\n", m_config->deviceID, m_currentMotorSetting, motorPinSetting);
	m_currentMotorSetting = motorPinSetting;
}

void TurnoutHandler::setConfig(TurnoutConfigStruct *value)
{ 
	m_config = value;
}
