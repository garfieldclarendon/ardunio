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
	void process(ModuleData &moduleData, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override;
	void processUDPMessage(ModuleData &moduleData, const UDPMessage &message, UDPMessage &outMessage, byte &messageIndex) override;

	void serverFound(UDPMessage &outMessage, byte &messageIndex) override;
	void processPin(byte pin, byte value) override;

private:
	bool parseConfig(String &jsonText, bool setVersion);
	bool m_downloadConfig;
	byte m_last;
	byte m_current;
	BlockState m_currentState;
	unsigned long m_currentTimeout;
};

