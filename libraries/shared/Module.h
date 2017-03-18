#pragma once

#include "Message.h"

class Module
{
public:
	Module(void);
	virtual ~Module(void) { }

	virtual byte setupWire(byte address) = 0;
	virtual bool process(byte &data) = 0;
	virtual bool handleMessage(const Message &message, byte &data) = 0;
	virtual bool getSendModuleState(void) const = 0;

	virtual byte getDeviceCount(void) const = 0;
	virtual short getDeviceID(byte index) const = 0;
	virtual byte getDeviceState(byte index) const = 0;
	virtual byte getCurrentState(void) const = 0;
	virtual void configCallback(const char *key, const char *value) = 0;
	virtual const char *getConfigReference(void) const = 0;
	virtual int getConfigSize(void) const = 0;

	byte getAddress(void) const { return m_address; }
protected:
	byte m_address;
};

