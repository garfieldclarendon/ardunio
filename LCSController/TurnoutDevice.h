#pragma once

#include "Device.h"
#include "GlobalDefs.h"

struct TurnoutRouteStruct
{
	int routeID;
	TurnoutState state;
};
typedef struct TurnoutRouteStruct TurnoutRouteStruct;

class TurnoutDevice : public Device
{
	const byte CONFIG_VERSION = 2;
public:
	TurnoutDevice();
	~TurnoutDevice();

	// Required Device overrides
	void process(ModuleData &data, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override;
	void processUDPMessage(ModuleData &data, const UDPMessage &message, UDPMessage &outMessage, byte &messageIndex) override;

	void controllerLockout(bool isLocked) {  }
	void networkOnline(void) override;
	void serverFound(UDPMessage &outMessage, byte &messageIndex) override;
	void setTurnout(byte &data, TurnoutState newState);

private:
	void setLockoutRoute(int value) { m_lockedRoute = value; }
	void sendStatusMessage(TurnoutState newState);
	bool parseConfig(String &jsonText, bool setVersion);
	TurnoutState readCurrentState(byte data);
	TurnoutState getTurnoutStateForRoute(int routeID);

	TurnoutState m_lastState;
	TurnoutState m_currentState;
	unsigned long m_currentTimeout;
	byte m_currentData;
	int m_lockedRoute;
	byte m_feedbackPinA;
	byte m_feedbackPinB;
	byte m_motorA;
	byte m_motorB;
	bool m_downloadConfig;
	byte m_motorBA;
	bool m_inputBA;
	TurnoutRouteStruct m_routeMap[MAX_ROUTE_ENTRIES];
};

