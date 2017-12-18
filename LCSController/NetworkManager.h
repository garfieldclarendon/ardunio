#pragma once

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <IPAddress.h>

#include "UDPMessage.h"
#include "GlobalDefs.h"

class WiFiClient;

struct NotificationStruct
{
	int controllerID;
	IPAddress address;
	NotificationStruct *next;
};
typedef struct NotificationStruct NotificationStruct;

struct MessageQueueStruct
{
	UDPMessage message;
	byte count;
	IPAddress address;
	MessageQueueStruct *next;
};
typedef struct MessageQueueStruct MessageQueueStruct;

class NetworkManager
{
public:
	enum WifiStatus
	{
		WiFiDisconnected,
		WiFiConnecting,
		WiFiConnected
	};
	typedef std::function<void(const UDPMessage &message)> TUDPMessageCallback;
	typedef std::function<void(bool connected)> TWIFIConnectionCallback;
	typedef std::function<void(const JsonArray &array)> TNotificationListChangedCallback;

	NetworkManager(void);
	~NetworkManager(void);

	void init(int serverPort, int controllerID);
	bool process(void);

	void setServerAddress(const IPAddress &value);
	IPAddress getServerAddress(void) const { return m_serverAddress;  }
	int getServerPort(void) const { return m_serverPort;  }
	bool getWiFiConnected(void) const;

	void sendUdpBroadcastMessage(const UDPMessage &message);
	void sendUdpMessage(const UDPMessage &message, bool addToQueue = true);
	bool sendUdpMessage(const UDPMessage &message, IPAddress &address, bool addToQueue);
	String getControllerConfig(unsigned int serialNumber);
	String getModuleConfig(unsigned int serialNumber, byte address);
	String getDeviceConfig(int deviceID);
	String getNotificationList(void);

	void setUdpMessageCallback(TUDPMessageCallback value) { m_udpMessageCallback = value;  }
	void setWIFIConnectCallback(TWIFIConnectionCallback value) { m_wifiConnectCallback = value; }
	void setNotificationListChangedCallback(TNotificationListChangedCallback value) { m_notificationListChangedCallback = value;  }

	void setControllerID(int value) { m_controllerID = value; }
	void setControllerName(const String &value) { m_controllerName = value; }
	void addNotificationController(int controllerID);
	bool updateNotificationList(int controllerID, const IPAddress &address);
	void setNotificationList(const String &jsonText);
	void clearNotificationList(void);

private:
	bool processWiFi(void);
	void processUDP(void);
	void checkNotificationList(void);
	void checkMessageQueue(void);
	String httpGet(const String &url);
	void getAddress(int controllerID);
	void addMessageToQueue(const UDPMessage &message, const IPAddress &address);
	void handleAckMessage(const UDPMessage &message);

	WiFiUDP m_udp;

	int m_serverPort;
	IPAddress m_serverAddress;
	static NetworkManager *m_this;
	String m_controllerName;
	int m_controllerID;
	unsigned long m_lastCheckTimeout;
	NotificationStruct *m_firstNotification;
	NotificationStruct *m_currentStruct;
	MessageQueueStruct *m_firstMessageQueue;
	WifiStatus m_wifiStatus;

	//Callbacks
	TUDPMessageCallback m_udpMessageCallback;
	TWIFIConnectionCallback m_wifiConnectCallback;
	TNotificationListChangedCallback m_notificationListChangedCallback;
};

extern NetworkManager NetManager;

