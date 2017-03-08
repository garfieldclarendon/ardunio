#pragma once

#include "Message.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"

class BlockHandler
{
public:
	enum BlockState
	{
		Unknown,
		Empty,
		Occupied
	};

	BlockHandler(void);
	void setup(byte blockPin);
	void setConfig(BlockConfigStruct value) { m_config = value; if (m_config.blockID > 0) m_currentState = Empty;  }
	BlockConfigStruct getConfig(void) const { return m_config; }
	bool process(byte &data);
	bool handleMessage(const Message &message);
	BlockState getCurrentState(void) const { return m_currentState; }

	int getBlockID(void) const { return m_config.blockID; }
	void setConfigValue(const char *key, const char *value);

private:
	byte m_blockPin;
	BlockConfigStruct m_config;
	BlockState m_currentState;
	byte m_current;
	byte m_lastRead;
	long m_currentTimeout;

};

