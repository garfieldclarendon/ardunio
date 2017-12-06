#pragma once

#include <ArduinoJson.h>
#include "UDPMessage.h"
#include "GlobalDefs.h"
#include "ModuleData.h"

class Device
{
public:
	Device();
	virtual ~Device();
	
	int getID(void) const { return m_id;  }
	void setID(int value) { m_id = value; }
	byte getPort(void) const { return m_port; }
	void setPort(byte value) { m_port = value;  }

	// Required overrides
	virtual void process(ModuleData &data) = 0;
	virtual void setup(int deviceID, byte port) = 0;
	virtual void sendStatusMessage(void) = 0;
	virtual void processUDPMessage(ModuleData &data, const UDPMessage &message) = 0;

	// Optional overrides
	virtual void controllerLockout(bool isLocked) { }
	virtual void networkOffline(void)  { }
	virtual void networkOnline(void)  { }
	virtual void processPin(byte pin, byte value) { }
	virtual String loadConfig(void) const;
	virtual void downloadConfig(void);
	virtual void saveConfig(const String &json);
	virtual void serverFound(void) { }

private:
	int m_id;
	byte m_port;
};

