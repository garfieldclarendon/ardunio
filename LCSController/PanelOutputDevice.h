#pragma once

#include "Device.h"
#include "GlobalDefs.h"

class PanelOutputDevice : public Device
{
	const byte CONFIG_VERSION = 2;
public:
	PanelOutputDevice();
	~PanelOutputDevice();
	// Required Device overrides
	void process(ModuleData &moduleData, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override { }
	void processUDPMessage(ModuleData &moduleData, const UDPMessage &message, UDPMessage &outMessage, byte &messageIndex) override;

	void serverFound(UDPMessage &outMessage, byte &messageIndex) override;

private:
	bool parseConfig(String &jsonText, bool setVersion);
	PinStateEnum getPinState(void);

	bool m_downloadConfig;
	int m_itemID;
	byte m_onValue;
	byte m_flashValue;
	byte m_currentValue;
};

