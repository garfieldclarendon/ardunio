#pragma once

#include "Device.h"
#include "GlobalDefs.h"

typedef struct TurnoutRouteStruct TurnoutRouteStruct;

struct TurnoutConfigStruct
{
	byte version;
	byte motorBA;
	byte inputBA;
};
typedef struct TurnoutConfigStruct TurnoutConfigStruct;

class TurnoutDevice : public Device
{
	const byte CONFIG_VERSION = 4;
public:
	TurnoutDevice();
	~TurnoutDevice();

	// Required Device overrides
	void process(ModuleData &data, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override;
	void processUDPMessage(ModuleData &data, const UDPMessage &message, UDPMessage &outMessage, byte &messageIndex) override;
	DeviceClassEnum getDeviceType(void) const { return DeviceTurnout; }

	void controllerLockout(bool isLocked) {  }
	void serverFound(UDPMessage &outMessage, byte &messageIndex) override;
	void setTurnout(byte &data, TurnoutState newState);
	byte getCurrentStatus(void) override { return m_currentState;  }

	String loadConfig(void);
	void saveConfig(const String &json);

private:
	void setLockoutRoute(int value) { m_lockedRoute = value; }
	void sendStatusMessage(TurnoutState newState);
	bool parseConfig(String &jsonText, bool setVersion);
	TurnoutState readCurrentState(byte data);
	void resetConfig(void);

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
	TurnoutConfigStruct m_data;
};

