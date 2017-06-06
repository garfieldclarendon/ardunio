#pragma once
#include <ArduinoJson.h>

#include "Message.h"
#include "Network.h"

// MCP23008 registers (everything except direction defaults to 0)
#define IODIR   0x00   // IO direction  (0 = output, 1 = input (Default))
#define IOPOL   0x01   // IO polarity   (0 = normal, 1 = inverse)
#define GPINTEN 0x02   // Interrupt on change (0 = disable, 1 = enable)
#define DEFVAL  0x03   // Default comparison for interrupt on change (interrupts on opposite)
#define INTCON  0x04   // Interrupt control (0 = interrupt on change from previous, 1 = interrupt on change from DEFVAL)
#define IOCON   0x05   // IO Configuration: bank/mirror/seqop/disslw/haen/odr/intpol/notimp
#define GPPU    0x06   // Pull-up resistor (0 = disabled, 1 = enabled)
#define INFTF   0x07   // Interrupt flag (read only) : (0 = no interrupt, 1 = pin caused interrupt)
#define INTCAP  0x08   // Interrupt capture (read only) : value of GPIO at time of last interrupt
#define GPIO    0x09   // Port value. Write to change, read to obtain value
#define OLLAT   0x0A   // Output latch. Write to latch output.

#define BASE_ADDRESS 0x20  // MCP23008 is on I2C port 0x20

class Module
{
public:
	Module(void);
	virtual ~Module(void) { }

	virtual void setup(void) { }
	virtual void setupWire(byte address) = 0;
	virtual bool processWire(void);
	virtual bool process(byte &data) = 0;

	virtual byte getIODirConfig(void) const = 0;
	virtual byte getDeviceCount(void) const = 0;
	virtual short getDeviceID(byte index) const = 0;
	virtual byte getCurrentState(void) const = 0;
	virtual void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json, byte &data) = 0;
	virtual void netModuleConfigCallback(NetActionType action, byte moduleIndex, const JsonObject &json) = 0;
	virtual void sendStatusMessage(void) = 0;
	virtual void netModuleCallbackWire(NetActionType action, byte moduleIndex, const JsonObject &json);

	byte getAddress(void) const { return m_address; }

protected:
	void setAddress(byte value) { m_address = value; }
	void expanderBegin(void);
	void expanderWrite(const byte reg, const byte data);
	byte expanderRead(const byte reg);

private:
	byte m_address;
};

