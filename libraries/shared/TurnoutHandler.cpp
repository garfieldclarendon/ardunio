#include <Arduino.h>
#include "TurnoutHandler.h"

TurnoutHandler::TurnoutHandler(void)
	: m_motorAPin(0), m_motorBPin(0), m_feedbackAPin(0), m_feedbackBPin(0), m_currentTimeout(0), m_currentMotorSetting(0), m_currentFeedbackA(0), m_currentFeedbackB(0)
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

	if (bitRead(data, m_feedbackAPin) != m_currentFeedbackA)
	{
		m_currentFeedbackA = bitRead(data, m_feedbackAPin);
//		DEBUG_PRINT("FEEDBACK A PIN CHANGED: %d\n", m_currentFeedbackA);
		ret = true;
	}
	if (bitRead(data, m_feedbackBPin) != m_currentFeedbackB)
	{
		m_currentFeedbackB = bitRead(data, m_feedbackBPin);
//		DEBUG_PRINT("FEEDBACK B PIN CHANGED: %d\n", m_currentFeedbackB);
		ret = true;
	}
	return ret;
}

void TurnoutHandler::setTurnout(byte motorPinSetting)
{
	DEBUG_PRINT("SETTURNOUT: %d CURRENT STATE: %d NEW STATE: %d\n", m_config->turnoutID, m_currentMotorSetting, motorPinSetting);
	m_currentMotorSetting = motorPinSetting;
}

void TurnoutHandler::setConfig(TurnoutConfigStruct *value)
{ 
	m_config = value;
}
