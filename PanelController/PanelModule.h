// PanelModule.h

#ifndef _PANELMODULE_h
#define _PANELMODULE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "globaldefs.h"

class PanelModuleClass
{
 protected:


 public:
	 PanelModuleClass(void);
	void setup(byte address);
	void process(bool buttonPressed);
	byte getModuleAddress(void) const;
	byte getOutputState(void) const { return m_outputs; }
	void netModuleCallback(NetActionType action, const JsonObject &json);

private:
	void handleButtonPressed(byte buttonIndex);
	void updateOutputs(byte pinIndex, PinStateEnum newState);

	void addBlinkingPin(byte pin);
	void blinkPins(void);
	void removeBlinkingPin(byte pin);
	void expanderWrite(const byte reg, const byte data);
	void expanderWriteBoth(const byte reg, const byte data);
	byte expanderRead(const byte reg);

	byte m_moduleAddress;
	byte m_blinkingPins[8];
	byte m_inputs;
	byte m_outputs;

	unsigned long m_currentBlinkTimeout;
};

#endif

