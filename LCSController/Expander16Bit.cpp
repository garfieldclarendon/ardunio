#include <Wire.h>

#include "Expander16Bit.h"
#include "GlobalDefs.h"


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

Expander16Bit::Expander16Bit(void)
	: m_address(0)
{
}

Expander16Bit::Expander16Bit(byte address)
	: m_address(address)
{
}

void Expander16Bit::expanderBegin(byte ioDirA, byte ioDirB)
{
	// set port A direction
	expanderWrite(IODIRA, ioDirA);
	// set port B direction
	expanderWrite(IODIRB, ioDirB);
}

byte Expander16Bit::readA(void)
{
	return expanderRead(GPIOA);
}

byte Expander16Bit::readB(void)
{
	return expanderRead(GPIOB);
}

void Expander16Bit::writeA(const byte data)
{
	expanderWrite(GPIOA, data);
}

void Expander16Bit::writeB(const byte data)
{
	expanderWrite(GPIOB, data);
}

void Expander16Bit::expanderWrite(const byte reg, const byte data)
{
	//	DEBUG_PRINT("expanderWrite: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, data);
	Wire.beginTransmission(BASE_ADDRESS | getAddress());
	Wire.write(reg);
	Wire.write(data);
	byte r = Wire.endTransmission(true);
	if (r != 0)
	{
		DEBUG_PRINT("expanderWrite16: Address: %d Register: %d  Data: %d FAILED! ERROR: %d\n", getAddress(), reg, data, r);
		if (I2C_ClearBus() == 0)
			Wire.begin(4, 5);
		else
			ESP.restart();
	}
}

void Expander16Bit::expanderWriteBoth(const byte reg, const byte data)
{
	Wire.beginTransmission(BASE_ADDRESS | getAddress());
	Wire.write(reg);
	Wire.write(data);  // port A
	Wire.write(data);  // port B
	byte r = Wire.endTransmission(true);
	if (r != 0)
	{
		DEBUG_PRINT("expanderWrite16: Address: %d Register: %d  Data: %d FAILED! ERROR: %d\n", getAddress(), reg, data, r);
		if (I2C_ClearBus() == 0)
			Wire.begin(4, 5);
		else
			ESP.restart();
	}
}
byte Expander16Bit::expanderRead(const byte reg)
{
	byte ret(0);

	Wire.beginTransmission(BASE_ADDRESS | getAddress());
	Wire.write(reg);
	byte r = Wire.endTransmission(true);
	if (r == 0)
	{
		Wire.requestFrom((byte)(BASE_ADDRESS | getAddress()), (byte)1);
		ret = Wire.read();
	}
	else
	{
//		DEBUG_PRINT("expanderRead: Address: %d Register: %d  FAILED! ERROR: %d\n", getAddress(), reg, r);
	}
	//	DEBUG_PRINT("expanderRead: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, ret);

	return ret;
}

int Expander16Bit::I2C_ClearBus(void)
{
	DEBUG_PRINT("I2C_ClearBus: Address: %d\n", getAddress());
#if defined(TWCR) && defined(TWEN)
	TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

	pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
	pinMode(SCL, INPUT_PULLUP);

	delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
	// up of the DS3231 module to allow it to initialize properly,
	// but is also assists in reliable programming of FioV3 boards as it gives the
	// IDE a chance to start uploaded the program
	// before existing sketch confuses the IDE by sending Serial data.

	boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
	if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
		return 1; //I2C bus error. Could not clear SCL clock line held low
	}

	boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
	int clockCount = 20; // > 2x9 clock

	while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
		clockCount--;
		// Note: I2C bus is open collector so do NOT drive SCL or SDA high.
		pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
		pinMode(SCL, OUTPUT); // then clock SCL Low
		delayMicroseconds(10); //  for >5uS
		pinMode(SCL, INPUT); // release SCL LOW
		pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
		// do not force high as slave may be holding it low for clock stretching.
		delayMicroseconds(10); //  for >5uS
		// The >5uS is so that even the slowest I2C devices are handled.
		SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
		int counter = 20;
		while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
			counter--;
			delay(100);
			SCL_LOW = (digitalRead(SCL) == LOW);
		}
		if (SCL_LOW) { // still low after 2 sec error
			return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
		}
		SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
	}
	if (SDA_LOW) { // still low
		return 3; // I2C bus error. Could not clear. SDA data line held low
	}

	// else pull SDA line low for Start or Repeated Start
	pinMode(SDA, INPUT); // remove pullup.
	pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
	// When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
	/// A Repeat Start is a Start occurring after a Start with no intervening Stop.
	delayMicroseconds(10); // wait >5uS
	pinMode(SDA, INPUT); // remove output low
	pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
	delayMicroseconds(10); // x. wait >5uS
	pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
	pinMode(SCL, INPUT);
	return 0; // all ok
}