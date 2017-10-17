#pragma once

#include "Module.h"
#include "GlobalDefs.h"
#include "ConfigStructures.h"
#include "SignalHandler.h"

class SignalModule : public Module
{
public:
	SignalModule(void);
	void setup(byte index, byte pin1, byte pin2, byte pin3);
	String createCurrentStatusJson(void);

	// Module overrides
	byte getIODirConfig(void) const override;
	byte getDeviceCount(void) const override { return MAX_SIGNALS; }
	short getDeviceID(byte index) const override { return 0; }
	byte getCurrentState(void) const override { return m_currentState; }
	void setup(void) override;
	void setupWire(byte address) override;
	bool process(byte &data) override;
	void sendStatusMessage(void) override;

	void netModuleCallback(NetActionType action, byte moduleIndex, const JsonObject &json, byte &data);
	void netModuleConfigCallback(NetActionType action, byte moduleIndex, const JsonObject &json);

private:
	void setSignal(byte port, PinStateEnum pin1State, PinStateEnum pin2State, PinStateEnum pin3State);
	SignalHandler m_signals[MAX_SIGNALS];

	byte m_currentState;
};

