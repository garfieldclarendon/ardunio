#include <Wire.h>

#include "Module.h"

Module::Module(void)
	: m_address(0)
{
}

bool Module::processWire(void)
{
	byte data = expanderRead(GPIO);
	byte hold = data;
	bool ret = process(data);
	
	if (data != hold)
		expanderWrite(GPIO, data);

	//byte io = expanderRead(IODIR);
	//if (io != getIODirConfig())
	//{
	//	DEBUG_PRINT("Module::processWire: ADDRESS: %d IO SETTINGS CORRUPTED!!!!   RESETTING\n", getAddress());
	//	expanderWrite(IODIR, getIODirConfig());
	//}

//	DEBUG_PRINT("Module::processWire: Address: %d data: %d  hold: %d io %d\n", getAddress(), data, hold, io);
	
	return ret;
}

void Module::netModuleCallbackWire(NetActionType action, byte moduleIndex, const JsonObject &json)
{
	byte data = expanderRead(GPIO);
	byte hold = data;
	netModuleCallback(action, moduleIndex, json, data);
	if (data != hold)
		expanderWrite(GPIO, data);
}
void Module::expanderBegin(void)
{
	Wire.beginTransmission(BASE_ADDRESS | getAddress());
	Wire.write((byte)IODIR);
	Wire.write((byte)0xFF);  // all inputs
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.write((byte)0x00);
	Wire.endTransmission();
}

void Module::expanderWrite(const byte reg, const byte data)
{
	//	DEBUG_PRINT("expanderWrite: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, data);
	Wire.beginTransmission(BASE_ADDRESS | getAddress());
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission();
}

byte Module::expanderRead(const byte reg)
{
	Wire.beginTransmission(BASE_ADDRESS | getAddress());
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom((byte)(BASE_ADDRESS | getAddress()), (byte)1);
	byte ret = Wire.read();
	//	DEBUG_PRINT("expanderRead: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, ret);

	return ret;
}
