#include <Wire.h>

#include "Expander8Bit.h"
#include "GlobalDefs.h"


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

Expander8Bit::Expander8Bit()
	: m_address(0), m_retryCount(0)
{
}

void Expander8Bit::expanderBegin(byte ioDir)
{
	m_retryCount = 0;
	expanderWrite(IODIR, ioDir);
}

void Expander8Bit::write(const byte data)
{
	expanderWrite(GPIO, data);
}

byte Expander8Bit::read(void)
{
	return expanderRead(GPIO);
}

void Expander8Bit::expanderWrite(const byte reg, const byte data)
{
	//	DEBUG_PRINT("expanderWrite: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, data);
	if (m_retryCount < MAX_I2C_RETRY)
	{
		Wire.beginTransmission(BASE_ADDRESS | getAddress());
		Wire.write(reg);
		Wire.write(data);
		byte r = Wire.endTransmission(true);
		if (r != 0)
		{
			DEBUG_PRINT("expanderWrite8: Address: %d Register: %d  Data: %d FAILED! ERROR: %d  retry count %d\n", getAddress(), reg, data, r, m_retryCount);
			m_retryCount++;
			if (I2C_ClearBus() == 0)
				Wire.begin(4, 5);
			else
				m_retryCount = 99;
		}
		else
		{
			m_retryCount = 0;
		}
	}
}

byte Expander8Bit::expanderRead(const byte reg)
{
	byte ret(0);
	if (m_retryCount < MAX_I2C_RETRY)
	{
		Wire.beginTransmission(BASE_ADDRESS | getAddress());
		Wire.write(reg);
		byte r = Wire.endTransmission(true);
		if (r == 0)
		{
			Wire.requestFrom((byte)(BASE_ADDRESS | getAddress()), (byte)1);
			ret = Wire.read();
			m_retryCount = 0;
		}
		else
		{
			m_retryCount++;
		}
	}
	//	DEBUG_PRINT("expanderRead: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, ret);

	return ret;
}

int Expander8Bit::I2C_ClearBus(void)
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