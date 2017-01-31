#pragma once

#include "Message.h"

class Module
{
public:
	Module(void);
	virtual ~Module(void) { }

	virtual void setup(void) = 0;
	virtual bool process(byte &data) = 0;
	virtual bool handleMessage(const Message &message, byte &data) = 0;

private:
};

