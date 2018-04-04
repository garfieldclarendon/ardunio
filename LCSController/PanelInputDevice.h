#pragma once

#include "Device.h"

class PanelInputDevice : public Device
{
	const byte CONFIG_VERSION = 2;
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

private:
	bool parseConfig(String &jsonText, bool setVersion);
	bool m_downloadConfig;
	int m_routeID;
};

