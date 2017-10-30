#include <Arduino.h>
#include "BlockModule.h"

BlockModule::BlockModule(void)
	: m_currentState(0), m_currentBlockConfig(-1)

{
	memset(&m_config, 0, sizeof(BlockControllerConfigStruct));
}

byte BlockModule::getIODirConfig(void) const
{
	byte iodir = 0;

	bitWrite(iodir, block1Pin, 1);
	bitWrite(iodir, block2Pin, 1);

	return iodir;
}

void BlockModule::setup(void)
{
	setup(0, block1Pin);
	setup(1, block2Pin);

	m_currentState = 0;
}

void BlockModule::setupWire(byte address)
{
	setAddress(address);
	byte iodir = getIODirConfig();

	setup(0, block1Pin);
	setup(1, block2Pin);

	m_blocks[0].setConfig(m_config.block1);
	m_blocks[1].setConfig(m_config.block2);

	expanderWrite(IODIR, iodir);
	delay(100);
	byte data = getCurrentState();
	expanderWrite(GPIO, data);
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
//        DEBUG_PRINT("process:  CURRENT_STATE %d === %d\n  STATUS_1 %d  STATUS_2 %d\n", m_currentState, data, m_blocks[0].getCurrentState(), m_blocks[1].getCurrentState());

	if (m_currentState != data)
	{
                DEBUG_PRINT("process:  CURRENT_STATE %d != %d\n  STATUS_1 %d  STATUS_2 %d\n", m_currentState, data, m_blocks[0].getCurrentState(), m_blocks[1].getCurrentState());
		m_currentState = data;
	}

	bool ret = false;
	if (sendStatus)
	{
		sendStatusMessage();
		ret = true;
	}

	return ret;
}


void BlockModule::netModuleCallback(NetActionType action, byte address, const JsonObject &json, byte &data)
{
	createCurrentStatusJson();
}

void BlockModule::netModuleConfigCallback(NetActionType /* action */, byte /* address */, const JsonObject & /* json */)
{
}

void BlockModule::sendStatusMessage(void)
{
	createCurrentStatusJson();
	//	DEBUG_PRINT("sendStatusMessage:  %s\n", json.c_str());
	//	Network.sendUdpMessageToServer(NetActionUpdate, ClassTurnout, getAddress(), json);
}

String BlockModule::createCurrentStatusJson(void)
{
	String json;
	StaticJsonBuffer<256> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root["messageUri"] = "/controller/module";
	root["address"] = getAddress();
	root["class"] = (int)ClassBlock;
	root["action"] = (int)NetActionUpdate;
	JsonArray &blocks = root.createNestedArray("blocks");

	for (byte x = 0; x < MAX_BLOCKS; x++)
	{
		JsonObject& block = blocks.createNestedObject();
		block["port"] = x;
		block["blockPin"] = m_blocks[x].getCurrentState();
	}

	root.printTo(json);
	Network.sendMessageToServer(root);

	return json;
}
