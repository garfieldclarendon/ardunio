// PanelModule.h
#pragma once

#include "Module.h"

class OutputModule : public Module
{


 public:
	OutputModule(void);
	void setup(byte address);
	byte getModuleAddress(void) const;
	byte getOutputStateA(void) const { return m_outputA; }
	byte getOutputStateB(void) const { return m_outputB; }
	void setFlashAll(bool value) { m_flashAll = value; }

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
	void serverOffline(void) override { m_flashAll = true; }
	void serverOnline(void) override { m_flashAll = false; }

private:
	void updateOutputs(byte pinIndex, PinStateEnum newState);

	void addBlinkingPin(byte pin);
	void blinkPins(void);
	void removeBlinkingPin(byte pin);
	void expanderWrite(const byte reg, const byte data);
	void expanderWriteBoth(const byte reg, const byte data);
	byte expanderRead(const byte reg);

	byte m_moduleAddress;
	byte m_blinkingPins[16];
	byte m_outputA;
	byte m_outputB;
	bool m_flashAll;
	byte m_outputRegisterA;
	byte m_outputRegisterB;

	unsigned long m_currentBlinkTimeout;
};


