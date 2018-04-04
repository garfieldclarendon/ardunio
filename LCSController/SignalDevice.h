#pragma once

#include "Device.h"
#include "GlobalDefs.h"

struct AspectDownloadStruct
{
	int aspectID;
	AspectDownloadStruct *next;
};
typedef struct AspectDownloadStruct AspectDownloadStruct;

struct DeviceStateStruct
{
	int deviceID;
	byte state;
};
typedef struct DeviceStateStruct DeviceStateStruct;

struct SignalConditionStruct
{
	ConditionConnectionEnum connection;
	int deviceID;
	ConditionEnum operand;
	byte deviceState;
};
typedef struct SignalConditionStruct SignalConditionStruct;

struct SignalAspectStruct
{
	PinStateEnum redMode;
	PinStateEnum greenMode;
	PinStateEnum yellowMode;
	byte conditionCount;
	SignalConditionStruct conditions[MAX_SIGNAL_CONDITIONS];
};
typedef struct SignalAspectStruct SignalAspectStruct;

class SignalDevice : public Device
{
	const byte CONFIG_VERSION = 5;
public:
	SignalDevice();
	virtual ~SignalDevice();

	// Required Device overrides
	void process(ModuleData &moduleData, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override { }
	void processUDPMessage(ModuleData &moduleData, const UDPMessage &message, UDPMessage &outMessage, byte &messageIndex) override;

	void serverFound(UDPMessage &outMessage, byte &messageIndex) override;
	void controllerLockout(bool isLocked) { m_lockout = isLocked; }

	PinStateEnum getRedMode(void) const { return m_redMode;  }
	PinStateEnum getGreenMode(void) const { return m_greenMode; }
	PinStateEnum getYellowMode(void) const { return m_yellowMode; }

protected:
	bool parseConfig(String &jsonText, bool setVersion);
	bool m_downloadConfig;

private:
	void setPin(byte &data, byte pin, PinStateEnum state);
	void updateValues(void);
	byte getCurrentState(int deviceID) const;
	bool loadAspect(byte index, SignalAspectStruct *aspect);
	void saveAspect(byte index, const SignalAspectStruct *aspect);
	String createFileName(byte index);
	void setInvalidAspect(void);
	void downloadConfig(void);
	void downloadAspects(void);
	void downloadAspect(int aspectID, byte index);
	void handleLockoutMessage(const UDPMessage &message);
	bool isNextConditionOR(SignalAspectStruct *aspect, byte nextIndex);

	PinStateEnum m_redMode;
	PinStateEnum m_greenMode;
	PinStateEnum m_yellowMode;

	bool m_updateValues;
	bool m_lockout;
	unsigned long m_currentBlinkTimeout;
	int m_blinkingTimeout;
	byte m_aspectCount;
	DeviceStateStruct m_deviceStates[MAX_SIGNAL_DEVICES];
	AspectDownloadStruct *m_aspectDownload;
};

