// 
// 
// 
#include <Wire.h>

#include "PanelModule.h"

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

PanelModuleClass::PanelModuleClass(void)
	: m_moduleAddress(0), m_inputs(0), m_outputs(0), m_currentBlinkTimeout(0), m_configIndex(-1)
{
	memset(&m_blinkingPins, 255, 8);
	memset(&m_configuration, 0, sizeof(ModuleConfigStruct));
}

void PanelModuleClass::setup(byte address)
{
	m_moduleAddress = BASE_ADDRESS + address;

	DEBUG_PRINT("setup module: %d\n", address);

	// set port A to output
	expanderWrite(IODIRA, 00);
	// set port B to input
	expanderWrite(IODIRB, 0xFF);
	/* //The interupt pin is connected to the expander chip, however, we are not using it at this time
	expanderWriteBoth(IOCON, 0b01100000); // mirror interrupts, disable sequential mode
	// enable all interrupts
	expanderWriteBoth(GPINTENA, 0xFF); // enable interrupts - both ports

	// read from interrupt capture ports to clear them
	expanderRead(INTCAPA);
	expanderRead(INTCAPB);
	*/
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

Message PanelModuleClass::process(bool buttonPressed)
{
	Message message;

	byte previousOutputs = m_outputs;
	byte buttons = expanderRead(GPIOB);
	if (buttons != m_inputs)
	{
		DEBUG_PRINT("Module::process  Module Address: %d\nButton values: %02x\n", m_moduleAddress - BASE_ADDRESS, buttons);
		m_inputs = buttons;

		for (byte index = 0; index < MAX_PANEL_INPUTS; index++)
		{
			if (bitRead(m_inputs, index) == LOW)
			{
				message = handleButtonPressed(index);
				break;
			}
		}
	}
	
	blinkPins();
	if (m_outputs != previousOutputs)
		expanderWrite(GPIOA, m_outputs);

	return message;
}

Message PanelModuleClass::handleButtonPressed(byte buttonIndex)
{
	DEBUG_PRINT("Module::handleButtonPressed  Module Address: %d\nButton index: %d\nRouteID:%d\n", m_moduleAddress - BASE_ADDRESS, buttonIndex, m_configuration.inputs[buttonIndex].id);

	Message message;

	if (m_configuration.inputs[buttonIndex].id > 0)
	{
		message.setDeviceID(m_configuration.inputs[buttonIndex].id);
		message.setMessageID(PANEL_ACTIVATE_ROUTE);
		message.setMessageClass(ClassRoute);
		message.setField(0, m_configuration.inputs[buttonIndex].value);
	}

	return message;
}

Message PanelModuleClass::handleMessage(const Message &message)
{
	int messageID = message.getMessageID();

	if (messageID == TRN_STATUS)
	{
		processSwitchTurnoutMessage(message);
		expanderWrite(GPIOA, m_outputs);
	}
	else if (messageID == BLOCK_STATUS)
	{
		processBlockMessage(message);
		expanderWrite(GPIOA, m_outputs);
	}

	Message returnMessage;
	return returnMessage;
}

void PanelModuleClass::processBlockMessage(const Message &message)
{
	byte newState = message.getField(0);
	int itemID = message.getDeviceID();

	DEBUG_PRINT("Block Status Message for: %d New State: %d\n", itemID, newState);

	updateOutputs(itemID, newState);
}

void PanelModuleClass::processSwitchTurnoutMessage(const Message &message)
{
	byte newState;
	int turnoutID;
	for (byte x = 0; x < MAX_MODULES; x++)
	{
		turnoutID = message.getDeviceStatusID(x);
		newState = message.getDeviceStatus(x);
		if (turnoutID == 0)
			break;
		DEBUG_PRINT("Turnout Status Message for: %d New State: %d\n", turnoutID, newState);
		updateOutputs(turnoutID, newState);
	}
}

void PanelModuleClass::updateOutputs(int itemID, byte newState)
{
	for (byte index = 0; index < MAX_PANEL_OUTPUTS; index++)
	{
		if (m_configuration.outputs[index].itemID == itemID && itemID > 0)
		{
			if (m_configuration.outputs[index].flashingValue == newState)
			{
				//Serial.print(index);
				//DEBUG_PRINT("%d Adding to blinking pins", index);
				addBlinkingPin(index);
			}
			else
			{
				DEBUG_PRINT("Index %1 State: %2 OnValue: %3\n", index, newState, m_configuration.outputs[index].onValue);
				removeBlinkingPin(index);
				bitWrite(m_outputs, index, m_configuration.outputs[index].onValue == newState);
			}
		}
	}
}

void PanelModuleClass::addBlinkingPin(byte pin)
{
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
	if (t - m_currentBlinkTimeout > blinkingTimeout)
	{
		m_currentBlinkTimeout = t;
		for (int x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] != 255)
			{
				int state = bitRead(m_outputs, m_blinkingPins[x]);
				bitWrite(m_outputs, m_blinkingPins[x], state == 0);
			}
		}
	}
}

void PanelModuleClass::configCallback(const char *key, const char *value)
{
	if (strcmp(key, "ID") == 0 || strcmp(key, "END") == 0)
	{
		m_configIndex = -1;
	}
	else if (strcmp(key, "INID") == 0)
	{
		m_configIndex++;
		m_configuration.inputs[m_configIndex].id = atoi(value);
	}
	else if (strcmp(key, "INTYPE") == 0)
	{
		m_configuration.inputs[m_configIndex].inputType = atoi(value);
	}
	else if (strcmp(key, "INVALUE") == 0)
	{
		m_configuration.inputs[m_configIndex].value = atoi(value);
	}
	else if (strcmp(key, "ITEMID") == 0)
	{
		m_configIndex++;
		m_configuration.outputs[m_configIndex].itemID = atoi(value);
	}
	else if (strcmp(key, "ITEMTYPE") == 0)
	{
		m_configuration.outputs[m_configIndex].itemType = atoi(value);
	}
	else if (strcmp(key, "ON") == 0)
	{
		m_configuration.outputs[m_configIndex].onValue = atoi(value);
	}
	else if (strcmp(key, "FLASH") == 0)
	{
		m_configuration.outputs[m_configIndex].flashingValue = atoi(value);
	}
}
