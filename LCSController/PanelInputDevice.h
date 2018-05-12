#pragma once

#include "Device.h"

struct PanelInputDataStruct
{
	byte m_version;
	int m_routeID;
};
typedef struct PanelInputDataStruct PanelInputDataStruct;

class PanelInputDevice : public Device
{
	const byte CONFIG_VERSION = 3;
public:
	PanelInputDevice();
	~PanelInputDevice();
	// Required Device overrides
	void process(ModuleData &moduleData, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override { }
	void processUDPMessage(ModuleData &moduleData, const UDPMessage &message, UDPMessage &outMessage, byte &messageIndex) override;

	void serverFound(UDPMessage &outMessage, byte &messageIndex) override;
	void processPin(byte pin, byte value) override;
	DeviceClassEnum getDeviceType(void) const { return DevicePanelInput; }

	String loadConfig(void);
	void saveConfig(const String &json);

private:
	bool parseConfig(String &jsonText, bool setVersion);
	bool m_downloadConfig;
	PanelInputDataStruct m_data;
};

