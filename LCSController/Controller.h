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
	typedef std::function<void(void)> TSendStatusCallback;

	Controller(int localServerPort);
	~Controller();

	void setup(ControllerClassEnum controllerClass, int controllerID);

	void setServerFoundCallback(TServerFoundCallback value) { m_serverFoundCallback = value; }
	void setSendStatusCallback(TSendStatusCallback value) { m_sendStatusCallback = value;  }
	ControllerClassEnum getClass(void) const { return m_class; }

	void process(void);
	void processMessage(const UDPMessage &message);
	void clearFiles(void);
	void restart(void);
	void networkOnline(void);
	void networkOffline(void);
	void resetConfiguration(void);

private:
	void downloadFirmwareUpdate(void);
	void sendControllerOnlineMessage(IPAddress &address);

	ControllerClassEnum m_class;
	int m_controllerID;

	TServerFoundCallback m_serverFoundCallback;
	TSendStatusCallback m_sendStatusCallback;
};

