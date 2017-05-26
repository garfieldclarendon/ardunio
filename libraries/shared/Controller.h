#pragma once
#include <IPAddress.h>
#include <functional>
#include <ArduinoJson.h>

#include "Message.h"
#include "GlobalDefs.h"
#include "ManageWiFi.h"

struct DeviceEntryStruct
{
	int deviceID;
	String deviceName;
};

class Controller
{
public:
	typedef std::function<void(void)> TWIFIReconnectCallback;

	Controller(int localServerPort);
	~Controller();

	void setup(ClassEnum controllerClass);
//	bool checkEEPROM(byte signature);

	int getControllerID(void) const { return m_controllerID;  }
	void setWIFIReconnectCallback(TWIFIReconnectCallback value) { m_wifiReconnectCallback = value; }
	ClassEnum getClass(void) const { return m_class; }
	void setClass(ClassEnum value) { m_class = value; }
	String getControllerName(void) const { return m_controllerName;  }
	void controllerCallback(NetActionType actionType, const JsonObject &root);
	void updateControllerName(const JsonObject &root);

	void process(void);
	void processMessage(const Message &message);
	bool getWiFiReconnected(void) const { return m_wifiManager.getIsReconnected(); }
	void clearFiles(void);
	void restart(void);

	byte getDeviceCount(void) const;
	int getDeviceID(byte index) const;
	String getDeviceName(byte index) const;

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

	short m_controllerID;
	String m_controllerName;
	ClassEnum m_class;
	ManageWiFi m_wifiManager;
	TWIFIReconnectCallback m_wifiReconnectCallback;
	DeviceEntryStruct m_devices[MAX_MODULES];
	byte m_extraPins[8];
	byte m_blinkingPins[8];
	unsigned long m_currentBlinkTimeout;
	unsigned long m_findServerTimeout;
	bool m_serverFound;
};

