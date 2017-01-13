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
	void setConfig(BlockConfigStruct value) { m_blockID = value.blockID; }
	bool process(void);
	bool handleMessage(const Message &message);
	BlockState getCurrentState(void) const { return m_currentState; }

	int getBlockID(void) const { return m_blockID; }

private:
	byte m_blockPin;
	int m_blockID;
	BlockState m_currentState;
	byte m_current;
	byte m_lastRead;
	long m_currentTimeout;

};

