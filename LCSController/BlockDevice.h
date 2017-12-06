#pragma once

#include "Device.h"
#include "GlobalDefs.h"

class BlockDevice : public Device
{
	const byte CONFIG_VERSION = 1;
public:
	BlockDevice();
	~BlockDevice();

	// Required Device overrides
	void process(ModuleData &moduleData) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override;
	void processUDPMessage(ModuleData &moduleData, const UDPMessage &message) override;

	void serverFound(void) override;
	void processPin(byte pin, byte value) override;

private:
	bool parseConfig(String &jsonText, bool setVersion);
	bool m_downloadConfig;
	BlockState m_currentState;
};

