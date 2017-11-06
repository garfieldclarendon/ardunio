#pragma once

#include "Module.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"
#include "TurnoutHandler.h"

class TurnoutModule : public Module
{
public:
	TurnoutModule(void);
	void setConfig(byte index, TurnoutConfigStruct value) 
	{ 
		if (index == 0)
			m_config.turnout1 = value;
		else if (index == 1)
			m_config.turnout2 = value;
	}
	TurnoutConfigStruct getConfig(byte index) const { return  m_turnouts[index].getConfig(); }

	void setup(byte index, byte motorAPin, byte motorBPin, byte feedbackAPin, byte feedbackBPin);
	void setTurnout(byte index, byte motorPinSetting);

	int getdeviceID(byte index) const { return m_turnouts[index].getdeviceID(); }
	String createCurrentStatusJson(void);

	// Module overrides
	byte getIODirConfig(void) const override;
	byte getCurrentState(void) const override { return m_currentState; }
	void setup(void) override;
	void setupWire(byte address) override;
	bool process(byte &data) override;
	void processNoWire(void) override;
	void sendStatusMessage(void);

	void netModuleCallbackNoWire(NetActionType action, byte address, const JsonObject &json);
	void netModuleCallback(NetActionType action, byte address, const JsonObject &json, byte &data);
	void netModuleConfigCallback(NetActionType action, byte address, const JsonObject &json);


private:
	void readPins(byte &data);
	void setPins(void);
	TurnoutHandler m_turnouts[MAX_TURNOUTS];

	byte m_currentState;
	TurnoutControllerConfigStruct m_config;

	const byte motorAPin1 = 0;
	const byte motorBPin1 = 1;
	const byte motorAPin2 = 4;
	const byte motorBPin2 = 5;
	const byte feedbackAPin1 = 2;
	const byte feedbackBPin1 = 3;
	const byte feedbackAPin2 = 6;
	const byte feedbackBPin2 = 7;

	// Turnout Controller pin assignments
	const byte motor1_pinA = 13;
	const byte motor1_pinB = 12;
	const byte motor2_pinA = 2;
	const byte motor2_pinB = 0;
	const byte normal2_pin = 4;
	const byte diverge2_pin = 5;
	const byte normal1_pin = 16;
	const byte diverge1_pin = 14;
};

