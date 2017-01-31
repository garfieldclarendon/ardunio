#pragma once

#include "Module.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"
#include "TurnoutHandler.h"

class TurnoutModule : public Module
{
public:
	TurnoutModule(void);
	void setup(void);
	void setup(byte index, byte motorAPin, byte motorBPin, byte normalPin, byte divergePin);
	void setConfig(byte index, TurnoutConfigStruct value) { m_turnouts[index].setConfig(value); }
	byte getTurnoutCount(void) const { return MAX_TURNOUTS;  }
	bool process(byte &data);
	bool handleMessage(const Message &message, byte &data);

	TurnoutState getCurrentTurnoutState(byte index) const { return m_turnouts[index].getCurrentState(); }
	void setTurnout(byte index, TurnoutState newTurnoutState, byte &data);

	int getTurnoutID(byte index) const { return m_turnouts[index].getTurnoutID(); }
	Message createMessage(void);
	byte getCurrentState(void) const { return m_currentState; }

private:
	TurnoutState getTurnoutStateForRoute(int routeID);
	TurnoutHandler m_turnouts[MAX_TURNOUTS];

	byte m_currentState;
};

