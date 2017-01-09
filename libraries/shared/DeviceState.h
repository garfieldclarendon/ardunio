#pragma once

#define MAX_DEVICE_STATES 100

class DeviceState
{
public:
  DeviceState(const DeviceState &other)
  {
    copy(other);  
  }

  DeviceState& operator=(const DeviceState &other) { copy(other); return *this; }

  static byte getDeviceState(int deviceID);
  static bool setDeviceState(int deviceID, byte currentState);
  static void addDevice(int deviceID);

private:
  DeviceState(void) : deviceID(0), currentState(0) { }
  DeviceState(int id, int state);

  int getDeviceID(void) const { return deviceID; }
  void setDeviceID(int value) { deviceID = value; }
  byte getCurrentState(void) const { return currentState; }
  void setCurrentState(byte value) { currentState = value; }
  void copy(const DeviceState &other);

  int deviceID;
  byte currentState;  
  static DeviceState deviceStates[MAX_DEVICE_STATES];
  static bool initialized;
};

