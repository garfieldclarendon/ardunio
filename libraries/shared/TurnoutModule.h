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

	void setup(byte index, byte motorAPin, byte motorBPin, byte normalPin, byte divergePin);
	TurnoutState getCurrentTurnoutState(byte index) const { return m_turnouts[index].getCurrentState(); }
	void setTurnout(byte index, TurnoutState newTurnoutState, byte &data);

	int getTurnoutID(byte index) const { return m_turnouts[index].getTurnoutID(); }
	Message createMessage(void);

	// Module overrides
	byte getDeviceCount(void) const override { return MAX_TURNOUTS; }
	short getDeviceID(byte index) const override { return getTurnoutID(index); }
	byte getDeviceState(byte index) const override { return m_turnouts[index].getCurrentState(); }
	byte getCurrentState(void) const override { return m_currentState; }
	byte setupWire(byte address) override;
	bool process(byte &data) override;
	bool handleMessage(const Message &message, byte &data) override;
	bool getSendModuleState(void) const override { return true; }
	void configCallback(const char *key, const char *value) override;
	const char *getConfigReference(void) const override;
	int getConfigSize(void) const override;
	TurnoutControllerConfigStruct getControllerConfigStruct(void) const { return m_config;  }

private:
	TurnoutState getTurnoutStateForRoute(int routeID);
	TurnoutHandler m_turnouts[MAX_TURNOUTS];

	byte m_currentState;
	byte m_currentTurnoutConfig;
	TurnoutControllerConfigStruct m_config;
};

