#pragma once
#include <IPAddress.h>
#include <functional>
#include <ArduinoJson.h>

#include "Message.h"
#include "GlobalDefs.h"
#include "ManageWiFi.h"

class Controller
{
public:
	typedef std::function<void(void)> TWIFIReconnectCallback;

	Controller(int localServerPort);
	~Controller();

	void setup(ClassEnum controllerClass);
//	bool checkEEPROM(byte signature);

	void setWIFIReconnectCallback(TWIFIReconnectCallback value) { m_wifiReconnectCallback = value; }
	ClassEnum getClass(void) const { return m_class; }
	void setClass(ClassEnum value) { m_class = value; }
	void controllerCallback(NetActionType actionType, const JsonObject &root);

	void process(void);
	void processMessage(const Message &message);
	bool getWiFiReconnected(void) const { return m_wifiManager.getIsReconnected(); }
	void clearFiles(void);
	void restart(void);

	void addExtraPin(byte virtualPin, byte physicalPin, PinModeEnum mode);
	void setExtraPin(byte virtualPin, PinStateEnum pinState);
	PinStateEnum getExtraPin(byte virtualPin);

private:
	void downloadFirmwareUpdate(void);
	void processLocalServer(void);
	void getServerAddress(IPAddress &address, int &port);
	void resetConfiguration(void);
	void findServer(void);
	void getControllerIDAndName(void);
	void addFlashingPin(byte pin);
	void removeFlashingPin(byte pin);
	void flashPins(void);

	ClassEnum m_class;
	ManageWiFi m_wifiManager;
	TWIFIReconnectCallback m_wifiReconnectCallback;
	byte m_extraPins[8];
	byte m_blinkingPins[8];
	unsigned long m_currentBlinkTimeout;
	unsigned long m_findServerTimeout;
	bool m_serverFound;
};

