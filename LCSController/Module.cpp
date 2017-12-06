#include "Module.h"
#include "Device.h"

Module::Module(void)
	: m_address(0)
{
	memset(m_devices, 0, sizeof(Device *) * MAX_DEVICES);
}

void Module::sendStatusMessage(void)
{
	for (byte x = 0; x < MAX_DEVICES; x++)
	{
		Device *device = getDevice(x);
		if (device)
			device->sendStatusMessage();
	}
}
