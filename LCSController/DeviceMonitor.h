#pragma once
#include "UDPMessage.h"

#include <Arduino.h>

#define TOTAL_DEVICES 128

class DeviceMonitor
{
public:
	DeviceMonitor();
	~DeviceMonitor();

	void processMessage(const UDPMessage &message);
	void addDevice(int deviceID);
	bool setDeviceStatus(int deviceID, byte status);

	byte getDeviceStatus(int deviceID) const;

private:
	int m_device[TOTAL_DEVICES];
	byte m_status[TOTAL_DEVICES];
};


extern DeviceMonitor Devices;

