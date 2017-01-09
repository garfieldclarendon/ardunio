#include <Arduino.h>
#include "DeviceState.h"


DeviceState DeviceState::deviceStates[MAX_DEVICE_STATES];
bool DeviceState::initialized = false;

DeviceState::DeviceState(int id, int state)
: deviceID(id), currentState(state)
{
    
}

byte DeviceState::getDeviceState(int deviceID)
{
	byte ret = 0;
	for (byte x = 0; x < MAX_DEVICE_STATES; x++)
	{
		if(deviceStates[x].getDeviceID() == deviceID)
		{
			ret = deviceStates[x].getCurrentState();
			break;
		}
	}
	return ret;
}

bool DeviceState::setDeviceState(int deviceID, byte currentState)
{
	bool anyUpdated = false;
	for(byte x = 0; x < MAX_DEVICE_STATES; x++)
	{
		if(deviceStates[x].getDeviceID() == deviceID)
		{
			deviceStates[x].setCurrentState(currentState);
			anyUpdated = true;
			break;
		}
	}
	return anyUpdated;
}

void DeviceState::addDevice(int deviceID)
{
	bool found = false;
	int firstIndex = -1;

	if (initialized == false)
	{
		memset(&deviceStates, 0, sizeof(DeviceState)*MAX_DEVICE_STATES);
		initialized = true;
	}

	if (deviceID > 0)
	{
		for (byte x = 0; x < MAX_DEVICE_STATES; x++)
		{
			if (deviceStates[x].getDeviceID() == deviceID)
			{
				found = true;
				break;
			}
			else if (deviceStates[x].getDeviceID() == 0 && firstIndex == -1)
			{
				firstIndex = x;
			}
		}

		if (found == false && firstIndex != -1)
			deviceStates[firstIndex].setDeviceID(deviceID);
	}
}

void DeviceState::copy(const DeviceState &other)
{
  deviceID = other.deviceID;
  currentState = other.currentState;
}

