// 
// OutputModule
// 
#include <Wire.h>
#include <ArduinoJson.h>

#include "InputModule.h"
#include "network.h"

// MCP23017 registers (everything except direction defaults to 0)
#define IODIRA   0x00   // IO direction  (0 = output, 1 = input (Default))
#define IODIRB   0x01
#define IOPOLA   0x02   // IO polarity   (0 = normal, 1 = inverse)
#define IOPOLB   0x03
#define GPINTENA 0x04   // Interrupt on change (0 = disable, 1 = enable)
#define GPINTENB 0x05
#define DEFVALA  0x06   // Default comparison for interrupt on change (interrupts on opposite)
#define DEFVALB  0x07
#define INTCONA  0x08   // Interrupt control (0 = interrupt on change from previous, 1 = interrupt on change from DEFVAL)
#define INTCONB  0x09
#define IOCON    0x0A   // IO Configuration: bank/mirror/seqop/disslw/haen/odr/intpol/notimp
//#define IOCON 0x0B  // same as 0x0A
#define GPPUA    0x0C   // Pull-up resistor (0 = disabled, 1 = enabled)
#define GPPUB    0x0D
#define INFTFA   0x0E   // Interrupt flag (read only) : (0 = no interrupt, 1 = pin caused interrupt)
#define INFTFB   0x0F
#define INTCAPA  0x10   // Interrupt capture (read only) : value of GPIO at time of last interrupt
#define INTCAPB  0x11
#define GPIOA    0x12   // Port value. Write to change, read to obtain value
#define GPIOB    0x13
#define OLLATA   0x14   // Output latch. Write to latch output.
#define OLLATB   0x15

#define BASE_ADDRESS 0x20  // MCP23017 is on I2C port 0x20

InputModule::InputModule(void)
	: m_moduleAddress(0), m_inputA(0), m_inputB(0), m_lockout(false)
{
}

byte InputModule::getModuleAddress(void) const 
{ 
	return m_moduleAddress - BASE_ADDRESS;
}

void InputModule::setup(void)
{
	m_inputRegisterA = GPIOA;
	m_inputRegisterB = GPIOB;

	m_inputA = 0;
	m_inputB = 0;
	m_moduleAddress = BASE_ADDRESS;

	DEBUG_PRINT("setup module: %d\n", m_moduleAddress);
}

void InputModule::setupWire(byte address)
{
	m_inputRegisterA = GPIOA;
	m_inputRegisterB = GPIOB;

	m_inputA = 0;
	m_inputB = 0;
	m_moduleAddress = BASE_ADDRESS + address;

	DEBUG_PRINT("InputModule: Setup module: %d\n", address);

	// set port A to output
	expanderWrite(IODIRA, 0xFF);
	// set port B to output
	expanderWrite(IODIRB, 0xFF);
	DEBUG_PRINT("InputModule::setupWire:  address %d DONE!!!!\n", address);
}

void InputModule::expanderWrite(const byte reg, const byte data)
{
	Wire.beginTransmission(m_moduleAddress);
	Wire.write(reg);
	Wire.write(data);  
	Wire.endTransmission();
}

void InputModule::expanderWriteBoth(const byte reg, const byte data)
{
	Wire.beginTransmission(m_moduleAddress);
	Wire.write(reg);
	Wire.write(data);  // port A
	Wire.write(data);  // port B
	Wire.endTransmission();
}

byte InputModule::expanderRead(const byte reg)
{
	Wire.beginTransmission(m_moduleAddress);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(m_moduleAddress, (byte)1);
	return Wire.read();
} 

bool InputModule::process(byte &)
{
	if (m_lockout == false)
	{
		// Read and process Port A
		byte input = expanderRead(m_inputRegisterA);
		if (input != m_inputA)
		{
			DEBUG_PRINT("InputModule::processA  Module Address: %d\Pin values: %02x\n", getModuleAddress(), input);
			m_inputA = input;

			for (byte index = 0; index < 16; index++)
			{
				if (bitRead(m_inputA, index) == LOW)
				{
					handleInput(index);
				}
			}
		}
		// Read and process Port B
		input = expanderRead(m_inputRegisterB);
		if (input != m_inputB)
		{
			DEBUG_PRINT("InputModule::processB  Module Address: %d\Pin values: %02x\n", getModuleAddress(), input);
			m_inputB = input;

			for (byte index = 0; index < 16; index++)
			{
				if (bitRead(m_inputB, index) == LOW)
				{
					handleInput(index + 8);
				}
			}
		}
	}

	return false;
}

void InputModule::handleInput(byte pin)
{
	DEBUG_PRINT("InputModule::handleInput  Module Address: %d\nPin index: %d\n", getModuleAddress(), pin);
	String json;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root["messageUri"] = "/controller/module";
	root["moduleIndex"] = getModuleAddress();
	root["class"] = (int)ClassInput;
	root["action"] = (int)NetActionUpdate;
	root["pin"] = pin;

	Network.sendMessageToServer(root);
}

void InputModule::netModuleCallbackWire(NetActionType action, byte , const JsonObject &)
{
	DEBUG_PRINT("InputModule::netModuleCallback  Action: %d\n", action);
}
