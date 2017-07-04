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
	String createCurrentStatusJson(void);

	// Module overrides
	byte getIODirConfig(void) const override;
	byte getDeviceCount(void) const override { return MAX_BLOCKS; }
	short getDeviceID(byte index) const override { return m_blocks[index].getBlockID(); }
	byte getCurrentState(void) const override { return m_currentState; }
	void setup(void) override;
	void setupWire(byte address) override;
	bool process(byte &data) override;
	void sendStatusMessage(void);

	void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json, byte &data);
	void netModuleConfigCallback(NetActionType action, byte moduleIndex, const JsonObject &json);

private:
	BlockHandler m_blocks[MAX_BLOCKS];

	byte m_currentState;
	byte m_currentBlockConfig;
	BlockControllerConfigStruct m_config;


	const byte block1Pin = 0;
	const byte block2Pin = 1;
};

