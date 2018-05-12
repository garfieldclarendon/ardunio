#include "DeviceMonitor.h"

DeviceMonitor::DeviceMonitor()
{
	for (byte x = 0; x < TOTAL_DEVICES; x++)
	{
		m_device[x] = 0;
		m_status[x] = 0;
	}
}

DeviceMonitor::~DeviceMonitor()
{
}

void DeviceMonitor::addDevice(int deviceID)
{
	for (byte x = 0; x < TOTAL_DEVICES; x++)
	{
		if (m_device[x] == deviceID)
		{
			break;
		}
		else if (m_device[x] == 0)
		{
			m_device[x] = deviceID;
			m_status[x] = 0;
			break;
		}
	}
}

bool DeviceMonitor::setDeviceStatus(int deviceID, byte status)
{
	bool ret(false);

	for (byte x = 0; x < TOTAL_DEVICES; x++)
	{
		if (m_device[x] == deviceID)
		{
			m_status[x] = status;
			ret = true;
			break;
		}
		else if (m_device[x] == 0)
		{
			break;
		}
	}
	return ret;
}

byte DeviceMonitor::getDeviceStatus(int deviceID) const
{
	byte status(0);

	for (byte x = 0; x < TOTAL_DEVICES; x++)
	{
		if (m_device[x] == deviceID)
		{
			status = m_status[x];
			break;
		}
		else if (m_device[x] == 0)
		{
			break;
		}
	}

	return status;
}

//-----------------------------------------------------------------------------------------------------------------------------//

DeviceMonitor Devices;