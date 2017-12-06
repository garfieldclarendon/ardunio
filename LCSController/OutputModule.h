// PanelModule.h
#pragma once

#include "Module.h"
#include "Expander16Bit.h"
#include "GlobalDefs.h"

class OutputModule : public Module
{

 public:
	OutputModule(void);
	~OutputModule(void) { }

	// Module overrides
	void setupWire(byte address) override;
	void processWire(void) override;
	void processUDPMessageWire(const UDPMessage &message) override;
	void processUDPMessageNoWire(const UDPMessage &message) override { }
	// This module is only supported on the I2CBus
	// So these functions do not apply
	void setupNoWire(void) override { }
	void processNoWire(void) override { }

private:

	Expander16Bit m_expander;
	byte m_outputA;
	byte m_outputB;

};


