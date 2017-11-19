#pragma once

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#include "UDPMessage.h"
#include "GlobalDefs.h"

class WiFiClient;

class NetworkManager
{
public:
	
	typedef std::function<void(const UDPMessage &message)> TUDPMessageCallback;
	typedef std::function<void(void)> TServerConnectedCallback;
	typedef std::function<void(void)> TServerDisconnectedCallback;
	typedef std::function<void(bool connected)> TWIFIConnectionCallback;

	NetworkManager(void);
	~NetworkManager(void);

	void init(int serverPort);
	bool process(void);

	void setServerAddress(const IPAddress &value);
	IPAddress getServerAddress(void) const { return m_serverAddress;  }
	int getServerPort(void) const { return m_serverPort;  }
	bool getWiFiConnected(void) const;

	void sendUdpBroadcastMessage(const UDPMessage &message);
	String getControllerConfig(unsigned int serialNumber);
	String getDeviceConfig(int deviceID);

	void setUdpMessageCallback(TUDPMessageCallback value) { m_udpMessageCallback = value;  }
	void setServerConnectedCallback(TServerConnectedCallback value) { m_serverConnectedCallback = value;  }
	void setWIFIConnectCallback(TWIFIConnectionCallback value) { m_wifiConnectCallback = value; }

	void setControllerID(int value) { m_controllerID = value; }
	void setControllerName(const String &value) { m_controllerName = value; }

	void handleUdpMessage(NetActionType action, ClassEnum moduleClass, const String &path, const String &payload);

private:
	bool processWiFi(void);
	void processUDP(void);
	String httpGet(const String &url);

	WiFiUDP m_udp;

	int m_serverPort;
	IPAddress m_serverAddress;
	static NetworkManager *m_this;
	String m_controllerName;
	int m_controllerID;
	unsigned long m_disconnectTimeout;

	//Callbacks
	TUDPMessageCallback m_udpMessageCallback;
	TServerConnectedCallback m_serverConnectedCallback;
	TWIFIConnectionCallback m_wifiConnectCallback;
};

extern NetworkManager NetManager;

