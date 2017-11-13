// PanelModule.h
#pragma once

#include "Module.h"

class InputModule : public Module
{


 public:
	InputModule(void);
	void setup(byte address);
	byte getModuleAddress(void) const;
	byte getOutputStateA(void) const { return m_inputA; }
	byte getOutputStateB(void) const { return m_inputB; }

	// Module overrides
	byte getIODirConfig(void) const override { return 0; }
	byte getCurrentState(void) const override { return 0; }
	void setup(void) override;
	void setupWire(byte address) override;
	bool process(byte &data) override;
	void sendStatusMessage(void) { };
	void netModuleCallback(NetActionType, byte, const JsonObject &, byte &) override  { }
	void netModuleConfigCallback(NetActionType, byte, const JsonObject &) override { }
	void netModuleCallbackWire(NetActionType action, byte moduleIndex, const JsonObject &json) override;
	void controllerLockout(bool locked) override { m_lockout = locked; }

private:
	void handleInput(byte pin, byte pinState);

	void expanderWrite(const byte reg, const byte data);
	void expanderWriteBoth(const byte reg, const byte data);
	byte expanderRead(const byte reg);

	byte m_moduleAddress;
	byte m_inputA;
	byte m_inputB;
	bool m_lockout;
	byte m_inputRegisterA;
	byte m_inputRegisterB;
};


