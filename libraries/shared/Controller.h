#pragma once
#include <WiFiUdp.h>
#include <functional>
#include <IPAddress.h>

#include "Message.h"
#include "GlobalDefs.h"

class Controller
{
public:
	typedef std::function<void(const Message &message)> TMessageHandlerFunction;

	Controller();
	~Controller();

	void setup(TMessageHandlerFunction messageCallback, ClassEnum controllerClass);
	bool checkEEPROM(byte signature);

	int getControllerID(void) const { return m_controllerID;  }
	void process(void);
	void sendNetworkMessage(const Message &message);
	ClassEnum getClass(void) const { return m_class; }
	void setClass(ClassEnum value) { m_class = value; }
	IPAddress getServerAddress(void) const { return m_serverAddress;  }
	byte getServerPort(void) const { return m_serverPort;  }

private:
	void setupNetwork(void);
	void handleSetControllerIDMessage(const Message &message);
	void handleServerHeartbeatMessage(const Message &message);
	void downloadFirmwareUpdate(void);

	TMessageHandlerFunction m_messageCallback;
	WiFiUDP m_udp;
	short m_controllerID;
	ClassEnum m_class;
	IPAddress m_serverAddress;
	byte m_serverPort;
};

