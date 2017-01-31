#pragma once
#include <WiFiUdp.h>
#include <functional>
#include <IPAddress.h>

#include "Message.h"
#include "GlobalDefs.h"
#include "ManageWiFi.h"

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
	void sendNetworkMessage(const Message &message, bool sendOnce = false);
	void sendUdpBroadcastMessage(const Message &message);
	ClassEnum getClass(void) const { return m_class; }
	void setClass(ClassEnum value) { m_class = value; }
	IPAddress getServerAddress(void) const { return m_serverAddress;  }
	byte getServerPort(void) const { return m_serverPort;  }
	bool getWiFiReconnected(void) const { return m_wifiManager.getIsReconnected(); }

private:
	void setupNetwork(void);
	void handleSetControllerIDMessage(const Message &message);
	void handleServerHeartbeatMessage(const Message &message);
	void downloadFirmwareUpdate(void);
	void processLocalServer(void);
	void processMessage(const Message &message);
	void resetSendMessageCounter(const Message &message);
	void resendLastMessage(void);
	void processUDP(void);

	TMessageHandlerFunction m_messageCallback;

	WiFiUDP m_udp;
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
	ManageWiFi m_wifiManager;
};

