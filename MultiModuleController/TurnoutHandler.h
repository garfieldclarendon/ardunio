#pragma once

#include "ConfigStructures.h"

class TurnoutHandler
{
public:
  TurnoutHandler(void);
  void setup(byte motorAPin, byte motorBPin, byte feedbackAPin, byte feedbackBPin);
  void setConfig(TurnoutConfigStruct *value);
  TurnoutConfigStruct getConfig(void) const { return *m_config; }
  bool process(byte &data);
  void setTurnout(byte motorPinSetting);

  int getdeviceID(void) const { return m_config->deviceID;  }
  byte getFeedbackA(void) const { return m_currentFeedbackA; }
  byte getFeedbackB(void) const { return m_currentFeedbackB; }

private:
	void readCurrentState(byte data);

	TurnoutConfigStruct *m_config;
	byte m_motorAPin;
	byte m_motorBPin;
	byte m_feedbackAPin;
	byte m_feedbackBPin;
	byte m_lastARead;
	byte m_lastBRead;
	byte m_currentMotorSetting;
	byte m_currentFeedbackA;
	byte m_currentFeedbackB;

	long m_currentATimeout;
	long m_currentBTimeout;
};

