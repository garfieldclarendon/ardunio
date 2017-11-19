#pragma once
#include <IPAddress.h>
#include <functional>
#include <ArduinoJson.h>

#include "UDPMessage.h"
#include "GlobalDefs.h"

class Controller
{
public:
	typedef std::function<void(void)> TServerFoundCallback;

	Controller(int localServerPort);
	~Controller();

	void setup(ClassEnum controllerClass);
//	bool checkEEPROM(byte signature);

	void setServerFoundCallback(TServerFoundCallback value) { m_serverFoundCallback = value; }
	ClassEnum getClass(void) const { return m_class; }
	void setClass(ClassEnum value) { m_class = value; }

	void process(void);
	void processMessage(const UDPMessage &message);
	void clearFiles(void);
	void restart(void);
	void findServer(void);

private:
	void downloadFirmwareUpdate(void);
	void getServerAddress(IPAddress &address, int &port);
	void resetConfiguration(void);
	void addFlashingPin(byte pin);
	void removeFlashingPin(byte pin);
	void flashPins(void);

	ClassEnum m_class;
	TServerFoundCallback m_serverFoundCallback;
	byte m_blinkingPins[8];
	unsigned long m_currentBlinkTimeout;
	unsigned long m_findServerTimeout;
	bool m_serverFound;
};

