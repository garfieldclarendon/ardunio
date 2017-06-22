// 
// 
// 
#include <Wire.h>
#include <ArduinoJson.h>

#include "PanelModule.h"
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

#define blinkingTimeout 200
#define slowBlinkingTimeout 1000

PanelModuleClass::PanelModuleClass(void)
	: m_moduleAddress(0), m_inputs(0), m_outputs(0), m_currentBlinkTimeout(0), m_flashAll(true)
{
	for (byte x = 0; x < 8; x++)
		m_blinkingPins[x] = 255;
}

byte PanelModuleClass::getModuleAddress(void) const 
{ 
	return m_moduleAddress - BASE_ADDRESS;
}

void PanelModuleClass::setup(byte address)
{
	if (address == 0)
	{
		m_inputRegister = GPIOB;
		m_outputRegister = GPIOA;
	}
	else
	{
		m_inputRegister = GPIOA;
		m_outputRegister = GPIOB;
	}
	m_currentBlinkTimeout = 0;
	m_outputs = 0;
	m_inputs = 0;
	m_flashAll = true;
	m_moduleAddress = BASE_ADDRESS + address;

	DEBUG_PRINT("setup module: %d\n", address);

	memset(m_blinkingPins, 255, 8);

	// set port A to output
	if (address == 0)
		expanderWrite(IODIRA, 00);
	else
		expanderWrite(IODIRA, 0xFF);
	// set port B to input
	if (address == 0)
		expanderWrite(IODIRB, 0xFF);
	else
		expanderWrite(IODIRB, 0);
}

void PanelModuleClass::expanderWrite(const byte reg, const byte data)
{
	Wire.beginTransmission(m_moduleAddress);
	Wire.write(reg);
	Wire.write(data);  
	Wire.endTransmission();
}

void PanelModuleClass::expanderWriteBoth(const byte reg, const byte data)
{
	Wire.beginTransmission(m_moduleAddress);
	Wire.write(reg);
	Wire.write(data);  // port A
	Wire.write(data);  // port B
	Wire.endTransmission();
}

byte PanelModuleClass::expanderRead(const byte reg)
{
	Wire.beginTransmission(m_moduleAddress);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(m_moduleAddress, (byte)1);
	return Wire.read();
} 

void PanelModuleClass::process(bool buttonPressed)
{
	byte previousOutputs = m_outputs;
	byte buttons = expanderRead(m_inputRegister);
	if (buttons != m_inputs)
	{
		DEBUG_PRINT("Module::process  Module Address: %d\nButton values: %02x\n", getModuleAddress(), buttons);
		m_inputs = buttons;

		for (byte index = 0; index < MAX_PANEL_INPUTS; index++)
		{
			if (bitRead(m_inputs, index) == LOW)
			{
				handleButtonPressed(index);
			}
		}
	}
	
	blinkPins();
	if (m_outputs != previousOutputs)
		expanderWrite(m_outputRegister, m_outputs);
}

void PanelModuleClass::handleButtonPressed(byte buttonIndex)
{
	DEBUG_PRINT("Module::handleButtonPressed  Module Address: %d\nButton index: %d\n", getModuleAddress(), buttonIndex);
	String json;
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root["messageUri"] = "/controller/module";
	root["moduleIndex"] = getModuleAddress();
	root["class"] = (int)ClassPanel;
	root["action"] = (int)NetActionUpdate;

	root["buttonIndex"] = buttonIndex;

	Network.sendMessageToServer(root);
}

void PanelModuleClass::updateOutputs(byte pinIndex, PinStateEnum newState)
{
	if (newState == PinFlashing)
	{
		//Serial.print(index);
		//DEBUG_PRINT("%d Adding to blinking pins", index);
		addBlinkingPin(pinIndex);
	}
	else
	{
		DEBUG_PRINT("Index %d State: %d\n", pinIndex, newState);
		removeBlinkingPin(pinIndex);
		bitWrite(m_outputs, pinIndex, newState);
		expanderWrite(m_outputRegister, m_outputs);
	}
}

void PanelModuleClass::addBlinkingPin(byte pin)
{
	DEBUG_PRINT("Adding to blinking pins: %d\n", pin);
	bool found = false;
	// Make sure the pin is not already in the list
	for (byte x = 0; x < 8; x++)
	{
		if (m_blinkingPins[x] == pin)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		// Add the pin to the first empty slot
		for (byte x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] == 255)
			{
				m_blinkingPins[x] = pin;
				break;
			}
		}
	}
}

void PanelModuleClass::removeBlinkingPin(byte pin)
{
	DEBUG_PRINT("Removing from blinking pins: %d\n", pin);
	for (byte x = 0; x < 8; x++)
	{
		if (m_blinkingPins[x] == pin)
		{
			m_blinkingPins[x] = 255;
		}
	}
}

void PanelModuleClass::blinkPins(void)
{
	unsigned long t = millis();

	if (m_flashAll)
	{
		// Flashing all doesn't work so well, so simply turn all LED's off
		// until we get re-connected to the server
		m_outputs = 0;
	}
	else
	{
		if (t - m_currentBlinkTimeout > blinkingTimeout)
		{
			m_currentBlinkTimeout = t;
			for (int x = 0; x < 8; x++)
			{
				if (m_blinkingPins[x] != 255)
				{
					int state = bitRead(m_outputs, m_blinkingPins[x]);
					bitWrite(m_outputs, m_blinkingPins[x], state == 0);
					//				DEBUG_PRINT("BLINKING PIN %d\n", m_blinkingPins[x]);
				}
			}
		}
	}
}

void PanelModuleClass::netModuleCallback(NetActionType action, const JsonObject &json)
{
	DEBUG_PRINT("PanelModuleClass::netModuleCallback  Action: %d\n", action);
	if (action == NetActionUpdate)
	{
		JsonArray &pins = json["pins"];

		for (byte x = 0; x < pins.size(); x++)
		{
			int pinIndex = pins[x]["pinIndex"];
			PinStateEnum pinState = (PinStateEnum)(byte)pins[x]["pinState"];

			updateOutputs(pinIndex, pinState);
		}
	}
}
