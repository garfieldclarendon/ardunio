#pragma once
#include "SignalDevice.h"

class SemaphoreDevice : public SignalDevice
{
	const byte CONFIG_VERSION = 1;
public:
	SemaphoreDevice();
	~SemaphoreDevice();
	void process(ModuleData &moduleData) override;
	void setup(int deviceID, byte port) override;

private:
//	bool parseConfig(String &jsonText, bool setVersion);
	byte m_motorA;
	byte m_motorB;
	byte m_motorBA;
};

