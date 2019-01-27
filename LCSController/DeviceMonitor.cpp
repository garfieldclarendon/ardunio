#include "DeviceMonitor.h"
#include "GlobalDefs.h"

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

void DeviceMonitor::processMessage(const UDPMessage & message)
{
	if (message.getMessageID() == TRN_STATUS || message.getMessageID() == BLK_STATUS)
	{
		setDeviceStatus(message.getMessageID(), message.getField(0));
	}
	else if (message.getMessageID() == DEVICE_STATUS)
	{
		byte index = 0;
		while (message.getDeviceID(index) > 0)
		{
			setDeviceStatus(message.getDeviceID(index), message.getDeviceStatus(index));
			index++;
		}
	}
}

void DeviceMonitor::addDevice(int deviceID)
{
	if (deviceID > 0)
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
			DEBUG_PRINT("DeviceMonitor::setDeviceStatus DEVICEID %d NEW STATUS: %d\n", deviceID, status);
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
	bool found = false;

	for (byte x = 0; x < TOTAL_DEVICES; x++)
	{
		if (m_device[x] == deviceID)
		{
			status = m_status[x];
			found = true;
			break;
		}
		else if (m_device[x] == 0)
		{
			break;
		}
	}

	if (!found)
	{
		DEBUG_PRINT("DeviceMonitor::getDeviceStatus DEVICEID %d NOT FOUND!!!!!!!!!!!!!!!!!!!!!\n", deviceID);
	}
	return status;
}

//-----------------------------------------------------------------------------------------------------------------------------//

DeviceMonitor Devices;