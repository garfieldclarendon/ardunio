#pragma once

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#include "Message.h"

class WiFiClient;

class NetworkClass
{
public:
	
	typedef std::function<void(int &controllerID, String &controllerName, String &controllerType, String &currentStatus)> TControllerStatusCallback;
	typedef std::function<void(NetActionType action, byte address, const JsonObject &json)> TModuleCallback;
	typedef std::function<void(NetActionType action, byte address, JsonObject &json)> TModuleConfigCallback;
	typedef std::function<String(NetActionType action, const JsonObject &json)> TControllerCallback;
	typedef std::function<String(NetActionType action, const JsonObject &json)> TControllerConfigCallback;
	typedef std::function<void(const Message &message)> TUDPMessageCallback;
	typedef std::function<void(void)> TServerConnectedCallback;
	typedef std::function<void(void)> TServerDisconnectedCallback;

	NetworkClass(void);
	~NetworkClass(void);

	void init(int serverPort);
	void process(void);

	void setServerAddress(const IPAddress &value);
	IPAddress getServerAddress(void) const { return m_serverAddress;  }
	int getServerPort(void) const { return m_serverPort;  }
	bool getIsConnected(void) const { return m_isConnected;  }

	void sendMessageToServer(const JsonObject &json);
//	void sendConfigMessageToServer(ClassEnum moduleClass, int address, const String &json, String &retJson);
//	void sendUdpMessageToServer(NetActionType action, ClassEnum moduleClass, int address, const String &json);
	void sendUdpBroadcastMessage(const Message &message);

	void setControllerStatusCallback(TControllerStatusCallback value) { m_controllerStatusCallback = value;  }
	void setModuleConfigCallback(TModuleConfigCallback value) { m_moduleConfigCallback = value; }
	void setModuleCallback(TModuleCallback value) { m_moduleCallback = value; }
	void setControllerCallback(TControllerCallback value) { m_controllerCallback = value; }
	void setControllerConfigCallback(TControllerCallback value) { m_controllerConfigCallback = value; }
	void setUdpMessageCallback(TUDPMessageCallback value) { m_udpMessageCallback = value;  }
	void setServerConnectedCallback(TServerConnectedCallback value) { m_serverConnectedCallback = value;  }

	void setControllerID(int value) { m_controllerID = value; }
	void setControllerName(const String &value) { m_controllerName = value; }

	void handleUdpMessage(NetActionType action, ClassEnum moduleClass, const String &path, const String &payload);

private:
	void processUDP(void);
	static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

	WiFiUDP m_udp;
	WebSocketsClient m_webSocket;

	int m_serverPort;
	IPAddress m_serverAddress;
	static NetworkClass *m_this;
	String m_controllerName;
	int m_controllerID;
	bool m_isConnected;
	unsigned long m_disconnectTimeout;

	//Callbacks
	TControllerStatusCallback m_controllerStatusCallback;
	TModuleConfigCallback m_moduleConfigCallback;
	TModuleCallback m_moduleCallback;
	TControllerCallback m_controllerCallback;
	TControllerConfigCallback m_controllerConfigCallback;
	TUDPMessageCallback m_udpMessageCallback;
	TServerConnectedCallback m_serverConnectedCallback;

	// Webserver handlers
	//static void handleRoot(void);
	//static void handleModuleConfig(void);
	//static void handleModule(void);
	//static void handleController(void);
	//static void handleControllerConfig(void);
};

extern NetworkClass Network;

