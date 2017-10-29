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

	int getTurnoutID(byte index) const { return m_turnouts[index].getTurnoutID(); }
	String createCurrentStatusJson(void);

	// Module overrides
	byte getIODirConfig(void) const override;
	byte getCurrentState(void) const override { return m_currentState; }
	void setup(void) override;
	void setupWire(byte address) override;
	bool process(byte &data) override;
	void sendStatusMessage(void);

	void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json, byte &data);
	void netModuleConfigCallback(NetActionType action, byte moduleIndex, const JsonObject &json);


private:
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

};

