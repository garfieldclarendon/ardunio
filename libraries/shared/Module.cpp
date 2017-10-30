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

//	DEBUG_PRINT("Module::processWire: Address: %d data: %d  hold: %d io %d\n", getAddress(), data, hold, io);
	
	return ret;
}

void Module::netModuleCallbackWire(NetActionType action, byte address, const JsonObject &json)
{
	byte data = expanderRead(GPIO);
	byte hold = data;
	netModuleCallback(action, address, json, data);
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
	byte r = Wire.endTransmission(true);
	if (r != 0)
	{
		DEBUG_PRINT("expanderWrite: Address: %d Register: %d  Data: %d FAILED! ERROR: %d\n", getAddress(), reg, data, r);
		if(I2C_ClearBus() == 0)
			Wire.begin(4, 5); 
		else
			ESP.restart();
	}
}

byte Module::expanderRead(const byte reg)
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
	//	DEBUG_PRINT("expanderRead: Address: %d Register: %d  Data: %d\n", moduleAddress, reg, ret);

	return ret;
}

int Module::I2C_ClearBus(void) 
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