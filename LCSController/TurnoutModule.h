#pragma once

#include "Module.h"
#include "Expander8Bit.h"
#include "NoWireHandler.h"

class TurnoutModule : public Module
{
public:
	TurnoutModule(void);
	// Module overrides
	void setupWire(byte address) override;
	void processWire(void) override;
	void processUDPMessageWire(const UDPMessage &message) override;
	void processUDPMessageNoWire(const UDPMessage &message) override;
	void setupNoWire(void) override;
	void processNoWire(void) override;
	void finishSetupWire(void) override;
	void finishSetupNoWire(void) override;
	void sendStatusMessage(void) override;

	const byte motorAPin1 = 0;
	const byte motorBPin1 = 1;
	const byte motorAPin2 = 4;
	const byte motorBPin2 = 5;
	const byte feedbackAPin1 = 2;
	const byte feedbackBPin1 = 3;
	const byte feedbackAPin2 = 6;
	const byte feedbackBPin2 = 7;

private:
	void processUDPMessage(byte &data, const UDPMessage &message);
	void readPins(byte &data);
	void setPins(void);
	byte getIODirConfig(void) const;

	byte m_currentState;
	Expander8Bit m_expander;
	NoWireHandler m_noWireHandler;

	// Turnout Controller pin assignments
	const byte motor1_pinA = 13;
	const byte motor1_pinB = 12;
	const byte motor2_pinA = 2;
	const byte motor2_pinB = 0;
	const byte normal2_pin = 4;
	const byte diverge2_pin = 5;
	const byte normal1_pin = 16;
	const byte diverge1_pin = 14;
};

