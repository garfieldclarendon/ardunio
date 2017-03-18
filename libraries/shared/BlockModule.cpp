#include <Arduino.h>
#include "BlockModule.h"

BlockModule::BlockModule(void)
	: m_currentState(0), m_currentBlockConfig(-1)

{
	memset(&m_config, 0, sizeof(BlockControllerConfigStruct));
}

byte BlockModule::setupWire(byte address)
{
	m_address = address;
	byte iodir = 0;
	byte block1Pin = 0;
	byte block2Pin = 1;

	bitWrite(iodir, block1Pin, 1);
	bitWrite(iodir, block2Pin, 1);

	setup(0, block1Pin);
	setup(1, block2Pin);

	m_blocks[0].setConfig(m_config.block1);
	m_blocks[1].setConfig(m_config.block2);

	return iodir;
}

void BlockModule::setup(byte index, byte blockPin)
{
	m_blocks[index].setup(blockPin);
}

bool BlockModule::process(byte &data)
{
	bool sendStatus = false;
	for (byte x = 0; x < MAX_BLOCKS; x++)
		if (m_blocks[x].process(data))
			sendStatus = true;
//	DEBUG_PRINT("process:  CURRENT_STATE %d === %d\n  STATUS_1 %d  STATUS_2 %d\n", m_currentState, data, m_blocks[0].getCurrentState(), m_blocks[1].getCurrentState());

	if (m_currentState != data)
	{
//		DEBUG_PRINT("process:  CURRENT_STATE %d != %d\n  STATUS_1 %d  STATUS_2 %d\n", m_currentState, data, m_blocks[0].getCurrentState(), m_blocks[1].getCurrentState());
		m_currentState = data;
	}

	bool ret = false;
	if (sendStatus)
	{
		ret = true;
	}

	return ret;
}

bool BlockModule::handleMessage(const Message &message, byte &data)
{
	bool ret = false;
	return ret;
}

Message BlockModule::createMessage(void)
{
	Message message;

	message.setMessageID(BLOCK_STATUS);
	message.setMessageClass(ClassBlock);
	message.setDeviceStatus(0, m_blocks[0].getBlockID(), m_blocks[0].getCurrentState());
	message.setDeviceStatus(1, m_blocks[1].getBlockID(), m_blocks[1].getCurrentState());
	return message;
}

void BlockModule::configCallback(const char *key, const char *value)
{
	if (strcmp(key, "ID") == 0)
	{
		m_currentBlockConfig++;
	}
	m_blocks[m_currentBlockConfig].setConfigValue(key, value);
	m_config.block1 = m_blocks[0].getConfig();
	m_config.block2 = m_blocks[1].getConfig();
}

const char *BlockModule::getConfigReference(void) const
{

	return (const char *)&m_config;
}

int BlockModule::getConfigSize(void) const
{
	return sizeof(BlockControllerConfigStruct);
}
