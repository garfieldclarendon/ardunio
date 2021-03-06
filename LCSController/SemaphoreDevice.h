#pragma once
#include "SignalDevice.h"

class SemaphoreDevice : public SignalDevice
{
	const byte CONFIG_VERSION = 1;
public:
	SemaphoreDevice();
	~SemaphoreDevice();
	void process(ModuleData &moduleData, UDPMessage &outMessage, byte &messageIndex) override;
	void setup(int deviceID, byte port) override;
	DeviceClassEnum getDeviceType(void) const { return DeviceSemaphore; }

private:
//	bool parseConfig(String &jsonText, bool setVersion);
	byte m_motorA;
	byte m_motorB;
	byte m_motorBA;
};

