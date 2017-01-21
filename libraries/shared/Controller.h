#pragma once
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <functional>
#include <IPAddress.h>

#include "Message.h"
#include "GlobalDefs.h"

class Controller
{
public:
	typedef std::function<void(const Message &message)> TMessageHandlerFunction;

	Controller(int localServerPort);
	~Controller();

	void setup(TMessageHandlerFunction messageCallback, ClassEnum controllerClass);
	bool checkEEPROM(byte signature);

	int getControllerID(void) const { return m_controllerID;  }
	void process(void);
//	void sendNetworkMessage(const Message &message, int deviceID);
//	void sendNetworkMessage(const Message &message, const String &service);
	void sendNetworkMessage(const Message &message, bool sendOnce = false);
	void sendUdpBroadcastMessage(const Message &message);
	ClassEnum getClass(void) const { return m_class; }
	void setClass(ClassEnum value) { m_class = value; }
	IPAddress getServerAddress(void) const { return m_serverAddress;  }
	byte getServerPort(void) const { return m_serverPort;  }

private:
	void setupNetwork(void);
	void setupMesh(void);
	void handleSetControllerIDMessage(const Message &message);
	void handleServerHeartbeatMessage(const Message &message);
	void downloadFirmwareUpdate(void);
	void processLocalServer(void);
	void processMessage(const Message &message);
	void updateDNSQuery(void);
	void resetSendMessageCounter(const Message &message);
	void resendLastMessage(void);

	TMessageHandlerFunction m_messageCallback;

	WiFiUDP m_udp;
	WiFiServer m_server;
	short m_controllerID;
	ClassEnum m_class;
	IPAddress m_serverAddress;
	byte m_serverPort;
	String m_lastDNSService;
	int m_lastDNSCount;
	long m_dnsCheckTimeout;
	byte m_resendMessageCount;
	long m_resendMessageTimeout;
	Message m_lastMessage;
};

