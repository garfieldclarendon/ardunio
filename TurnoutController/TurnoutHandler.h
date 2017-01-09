#pragma once

#include "Message.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"

class TurnoutHandler
{
public:
  TurnoutHandler(void);
  void setup(byte motorAPin, byte motorBPin, byte normalPin, byte divergePin);
  void setConfig(TurnoutConfigStruct value) { m_config = value;  }
  bool process(void);
  bool handleMessage(const Message &message);
  TurnoutState getCurrentState(void) const { return m_currentState;  }
  void setTurnout(TurnoutState newState);

  int getTurnoutID(void) const { return m_config.turnoutID;  }
  Message createMessage(TurnoutState newState);

private:
	TurnoutState getTurnoutStateForRoute(int routeID);
	TurnoutState readCurrentState(void);

	TurnoutConfigStruct m_config;
	byte m_motorAPin;
	byte m_motorBPin;
	byte m_normalPin;
	byte m_divergePin;

	TurnoutState m_currentState;
	TurnoutState m_lastState;
	long m_currentTimeout;
};

