#pragma once

#include "Device.h"
#include "GlobalDefs.h"

struct PanelOutputDataStruct
{
	byte m_version;
	int m_itemID;
	byte m_onValue;
	byte m_flashValue;
	byte m_routeCount;
};

typedef struct PanelOutputDataStruct PanelOutputDataStruct;

class PanelOutputDevice : public Device
{
	const byte CONFIG_VERSION = 5;
public:
	PanelOutputDevice();
	~PanelOutputDevice();
	// Required Device overrides
	void process(ModuleData &moduleData, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	void sendStatusMessage(void) override { }
	void processUDPMessage(ModuleData &moduleData, const UDPMessage &message, UDPMessage &outMessage, byte &messageIndex) override;
	DeviceClassEnum getDeviceType(void) const { return DevicePanelOutput; }

	void serverFound(UDPMessage &outMessage, byte &messageIndex) override;
	String loadConfig(void);
	void saveConfig(const String &json);

private:
	bool parseConfig(String &jsonText, bool setVersion);
	PinStateEnum getPinState(void);
	void updateCurrentStatus(void);

	bool m_downloadConfig;
	PanelOutputDataStruct m_data;
	byte m_currentStatus;
	int *m_routeList;
};

