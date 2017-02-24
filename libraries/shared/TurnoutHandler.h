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
  TurnoutConfigStruct getConfig(void) const { return m_config; }
  bool process(byte &data);
  TurnoutState getCurrentState(void) const { return m_currentState;  }
  void setTurnout(TurnoutState newState, byte &data);

  int getTurnoutID(void) const { return m_config.turnoutID;  }
  TurnoutState getTurnoutStateForRoute(int routeID);

  void setConfigValue(const char *key, const char *value);

private:
	TurnoutState readCurrentState(byte data);

	TurnoutConfigStruct m_config;
	byte m_motorAPin;
	byte m_motorBPin;
	byte m_normalPin;
	byte m_divergePin;

	TurnoutState m_currentState;
	TurnoutState m_lastState;
	long m_currentTimeout;
	byte m_currentRouteConfig;
};

