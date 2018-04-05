#pragma once

#include <Arduino.h>
#include "GlobalDefs.h"
#include "UDPMessage.h"

class Device;

class Module
{
public:
	Module(void);
	virtual ~Module(void) { }

	virtual void setupWire(byte address) = 0;
	virtual void processWire(void) = 0;

	virtual void setupNoWire(void) = 0;
	virtual void processNoWire(void) = 0;
	virtual void processUDPMessageWire(const UDPMessage &message) = 0;
	virtual void processUDPMessageNoWire(const UDPMessage &message) = 0;

	virtual void finishSetupWire(void) { }
	virtual void finishSetupNoWire(void) { }

	byte getAddress(void) const { return m_address; }
	void addDevice(byte index, Device *device) { m_devices[index] = device; }
	Device *getDevice(byte index) { return m_devices[index]; }
	virtual void sendStatusMessage(void);

protected:

	void setAddress(byte value) { m_address = value; }

private:
	Device *m_devices[MAX_DEVICES];
	byte m_address;
};

