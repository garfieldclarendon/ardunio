#pragma once

#include "Device.h"

class PanelInputDevice : public Device
{
	const byte CONFIG_VERSION = 2;
public:
	PanelInputDevice();
	~PanelInputDevice();
	// Required Device overrides
	void process(ModuleData &moduleData) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override { }
	void processUDPMessage(ModuleData &moduleData, const UDPMessage &message) override;

	void serverFound(void) override;
	void processPin(byte pin, byte value) override;

private:
	bool parseConfig(String &jsonText, bool setVersion);
	bool m_downloadConfig;
	int m_routeID;
};

