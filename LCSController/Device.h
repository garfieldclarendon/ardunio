#pragma once

#include <ArduinoJson.h>
#include "UDPMessage.h"
#include "GlobalDefs.h"

class Device
{
public:
	Device();
	virtual ~Device();
	
	int getID(void) const { return m_id;  }
	void setID(int value) { m_id = value; }
	byte getAddress(void) const { return m_address;  }
	void setAddress(byte value) { m_address = value; }
	byte getPort(void) const { return m_port; }
	void setPort(byte value) { m_port = value;  }
	Device *getNextDevice(void) const { return m_nextDevice; }
	void setNextDevice(Device *value) { m_nextDevice = value; }

	// Required overrides
	virtual void processWire(void) = 0;
	virtual void processNoWire(void) = 0;
	virtual void setupWire(byte address, byte port) = 0;
	virtual void setup(byte port) = 0;
	virtual void sendStatusMessage(void) = 0;
	virtual void processUDPMessage(const UDPMessage &message) = 0;

	// Optional overrides
	virtual void controllerLockout(bool isLocked) { }
	virtual void networkOffline(void)  { }
	virtual void networkOnline(void)  { }
	virtual void netDeviceConfigCallback(NetActionType action, byte address, byte port, const JsonObject &json) { }
	virtual String loadConfig(void) const;
	virtual void downloadConfig(void);
	virtual void saveConfig(const String &json);

private:
	int m_id;
	byte m_address;
	byte m_port;
	Device *m_nextDevice;
};

