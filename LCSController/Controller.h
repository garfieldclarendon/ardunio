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

	void setup(ControllerClassEnum controllerClass);

	void setServerFoundCallback(TServerFoundCallback value) { m_serverFoundCallback = value; }
	void setSendStatusCallback(TSendStatusCallback value) { m_sendStatusCallback = value;  }
	ControllerClassEnum getClass(void) const { return m_class; }
	void setClass(ControllerClassEnum value) { m_class = value; }
	void setControllerID(int value) { m_controllerID = value; }

	void process(void);
	void processMessage(const UDPMessage &message);
	void clearFiles(void);
	void restart(void);
	void networkOnline(void);
	void networkOffline(void);

private:
	void downloadFirmwareUpdate(void);
	void getServerAddress(IPAddress &address, int &port);
	void resetConfiguration(void);
	void sendControllerOnlineMessage(IPAddress &address);

	ControllerClassEnum m_class;
	TServerFoundCallback m_serverFoundCallback;
	TSendStatusCallback m_sendStatusCallback;
	bool m_serverFound;
	int m_controllerID;
};
