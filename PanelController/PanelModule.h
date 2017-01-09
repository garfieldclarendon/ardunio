// PanelModule.h

#ifndef _PANELMODULE_h
#define _PANELMODULE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Message.h"
#include "Structures.h"

class PanelModuleClass
{
 protected:


 public:
	 PanelModuleClass(void);
	void setup(byte address);
	Message process(bool buttonPressed);
	Message handleMessage(const Message &message);
	void setConfiguration(ModuleConfigStruct value) { m_configuration = value; }

private:
	Message handleButtonPressed(byte buttonIndex);
	void processSwitchTurnoutMessage(const Message &message);
	void processBlockMessage(const Message &message);
	void updateOutputs(int itemID, byte newState);

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
	ModuleConfigStruct m_configuration;
};

extern PanelModuleClass PanelModule;

#endif

