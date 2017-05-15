#pragma once

#include "Module.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"
#include "BlockHandler.h"

class BlockModule : public Module
{
public:
	BlockModule(void);
	void setConfig(byte index, BlockConfigStruct value) { m_blocks[index].setConfig(value); }
	BlockConfigStruct getConfig(byte index) const { return  m_blocks[index].getConfig(); }
	void setup(byte index, byte blockPin);

	// Module overrides
	byte getDeviceCount(void) const override { return MAX_BLOCKS; }
	short getDeviceID(byte index) const override { return m_blocks[index].getBlockID(); }
	byte getCurrentState(void) const override { return m_currentState; }
	void setupWire(byte address) override;
	bool process(byte &data) override;

private:
	BlockHandler m_blocks[MAX_BLOCKS];

	byte m_currentState;
	byte m_currentBlockConfig;
	BlockControllerConfigStruct m_config;
};

