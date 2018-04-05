// PanelModule.h
#pragma once

#include "Module.h"
#include "Expander16Bit.h"

class InputModule : public Module
{


 public:
	InputModule(void);
	~InputModule(void) { }
	
	// Module overrides
	void setupWire(byte address) override;
	void processWire(void) override;
	void processUDPMessageWire(const UDPMessage &message) override;
	void processUDPMessageNoWire(const UDPMessage &message) override { }
	void finishSetupWire(void) override;
	void sendStatusMessage(void) override;

	// This module is only supported on the I2CBus
	// So these functions do not apply
	void setupNoWire(void) override { }
	void processNoWire(void) override { }

private:
	void handleInput(byte pin, byte pinState);

	void expanderWrite(const byte reg, const byte data);
	void expanderWriteBoth(const byte reg, const byte data);
	byte expanderRead(const byte reg);

	Expander16Bit m_expander;
	byte m_moduleAddress;
	byte m_inputA;
	byte m_inputB;
};


