#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "NetworkManager.h"
#include "GlobalDefs.h"

NetworkManager *NetworkManager::m_this = NULL;
#define disconnectedTimeout 60000

NetworkManager::NetworkManager(void)
	: m_serverPort(-1), m_controllerID(0), m_serverAddress(0, 0, 0, 0), m_lastCheckTimeout(0), m_firstNotification(NULL), m_currentStruct(NULL)
{
	m_this = this;
}

NetworkManager::~NetworkManager(void)
{
}

void NetworkManager::setServerAddress(const IPAddress &value)
{ 
	if (m_serverAddress != value)
	{
		m_serverAddress = value;
	}
}

void NetworkManager::init(int serverPort, int controllerID)
{
	DEBUG_PRINT("init: %d\n", controllerID);

	m_serverPort = serverPort;
	m_controllerID = controllerID;

	WiFi.mode(WIFI_STA);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	wl_status_t status = WiFi.status();
	if (status == WL_CONNECTED)
	{
		if (m_wifiConnectCallback)
			m_wifiConnectCallback(true);
	}
	else
	{
		processWiFi();
	}
	if (m_udp.begin(UdpPort))
		DEBUG_PRINT("Now listening on %s, UDP port %d\n", WiFi.localIP().toString().c_str(), UdpPort);
	else
		DEBUG_PRINT("Error starting UDP!\n");
}

bool NetworkManager::process(void)
{
	bool ret = processWiFi();
	processUDP();
	checkNotificationList();
	return ret;
}

bool NetworkManager::processWiFi(void)
{
	bool wiFiReconnected = false;
	wl_status_t status = WiFi.status();
	//	DEBUG_PRINT("[WIFI] process::status %d\n", status);
	if (status == WL_DISCONNECTED || status == WL_IDLE_STATUS || status == WL_CONNECT_FAILED)
	{
		WiFi.begin(ssid, password);
		status = WiFi.status();

		// wait for connection or fail
		DEBUG_PRINT("[WIFI]connecting to %s.", ssid);
		while (status != WL_CONNECTED && status != WL_NO_SSID_AVAIL && status != WL_CONNECT_FAILED)
		{
			DEBUG_PRINT(".");
			delay(100);
			status = WiFi.status();
		}
		DEBUG_PRINT("[WIFI] Status: %d.\n", status);

		if (m_wifiConnectCallback)
			m_wifiConnectCallback(status == WL_CONNECTED);
	}
	return wiFiReconnected;
}

void NetworkManager::processUDP(void)
{
//	DEBUG_PRINT("processUDP!  From: %s\n", m_udp.remoteIP().toString().c_str());
	static bool signatureFound = false;
	int packetSize;
	if (signatureFound)
		packetSize = m_udp.available();
	else
		packetSize = m_udp.parsePacket();

	if (packetSize > 0)
	{
		DEBUG_PRINT("processUDP:  PACKET SIZE: %d\n", packetSize);
	}

	// Find the start of a valid message.
	// ...ignore everything else
	if (packetSize >= 0 && signatureFound == false)
	{
		while (m_udp.peek() != 0xEE && (packetSize = m_udp.available()) > 0)
			m_udp.read();
		if (m_udp.peek() == 0xEE)
		{
			m_udp.read();
			if (m_udp.peek() == 0xEF)
			{
				signatureFound = true;
				m_udp.read();
			}
		}
	}

	// wait until a full message comes in before processing
	if (signatureFound)
	{
		signatureFound = false;
//		DEBUG_PRINT("READING MESSAGE!\n");
		UDPMessage message;
		byte size = sizeof(UDPMessageStruct);
		char *ref = message.getRef();
		// Skip the signature bytes
		ref += 2;
		while (m_udp.available() > 0)
		{
			byte b = m_udp.read();
			if (size <= sizeof(UDPMessageStruct))
			{
				*ref = b;
				size--;
				ref++;
			}
			// If this is the end of message signature, remove any extra
			// data
			if (b == 0xEF && m_udp.peek() == 0xEE)
			{
				// read the second byte of the end of message signature (0XEE)
				b = m_udp.read();
//				DEBUG_PRINT("END Signature found!!\n");
				if (m_udp.available() > 0)
					b = m_udp.read();
				// If more data available, read to the next start signature
				while (m_udp.available() > 0 && b != 0xEE && m_udp.peek() != 0xEF)
					b = m_udp.read();
				break;
			}
		}

		if (m_udpMessageCallback)
			m_udpMessageCallback(message);
	}
}

void NetworkManager::sendUdpBroadcastMessage(const UDPMessage &message)
{
	IPAddress broadcastIp;
	broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();
	sendUdpMessage(message, broadcastIp);
}

void NetworkManager::sendUdpMessage(const UDPMessage &message)
{
	NotificationStruct *current = m_firstNotification;

	while (current)
	{
		if (current->address[0] != 0)
		{
			if (sendUdpMessage(message, current->address) == false)
				current->address = IPAddress();
		}
		current = current->next;
	}

	if (m_serverAddress[0] != 0)
	{
		sendUdpMessage(message, m_serverAddress);
	}

	// Give other modules connected to this controller a chance to process the message too
	if (m_udpMessageCallback)
		m_udpMessageCallback(message);
}

bool NetworkManager::sendUdpMessage(const UDPMessage &message, IPAddress &address)
{
	bool ret = false;
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	DEBUG_PRINT("MessageSize: %d  MessageID: %d ID: %d TO %s\n", sizeof(UDPMessageStruct), message.getMessageID(), message.getID(), address.toString().c_str());

	if (m_udp.beginPacket(address, UdpPort) == 0)
	{
		DEBUG_PRINT("sendUdpBroadcastMessage beginPacket failed!!!\n");
	}
	else
	{
		m_udp.write(message.getRef(), sizeof(UDPMessageStruct));
		if (m_udp.endPacket() == 0)
		{
			DEBUG_PRINT("sendUdpBroadcastMessage beginPacket failed!!!\n");
		}
		else
		{
			ret = true;
		}
	}
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return ret;
}

String NetworkManager::getControllerConfig(unsigned int serialNumber)
{
	String url("/controller/config?serialNumber=");
	url += serialNumber;
	String json = httpGet(url);
	return json;
}

String NetworkManager::getModuleConfig(unsigned int serialNumber, byte address)
{
	String url("/controller/module/config?serialNumber=");
	url += serialNumber;
	url += "&address=";
	url += address;
	String json = httpGet(url);
	return json;
}

String NetworkManager::getDeviceConfig(int deviceID)
{
	String url("/controller/device/config?deviceID=");
	url += deviceID;
	String json = httpGet(url);
	return json;
}

String NetworkManager::getNotificationList(void)
{
	String url("/controller/notification_list?serialNumber=");
	url += ESP.getChipId();
	String json = httpGet(url);
	return json;
}

String NetworkManager::httpGet(const String &url)
{
	HTTPClient http;

	// Build the full url including the port number
	String fullUrl("http://");
	fullUrl += getServerAddress().toString();
	fullUrl += ":";
	fullUrl += getServerPort();
	fullUrl += url;
	DEBUG_PRINT("[HTTP] GET begin...\n");
	DEBUG_PRINT("%s\n", fullUrl.c_str());
	http.begin(fullUrl);

	// start connection and send HTTP header
	int httpCode = http.GET();
	String payload;
	// httpCode will be negative on error
	if (httpCode > 0) {
		// HTTP header has been send and Server response header has been handled
		DEBUG_PRINT("[HTTP] GET... code: %d\n", httpCode);

		// file found at server
		if (httpCode == HTTP_CODE_OK) {
			payload = http.getString();
			DEBUG_PRINT("%s\n", payload.c_str());
		}
	}
	else 
	{
		DEBUG_PRINT("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();
	return payload;
}

bool NetworkManager::getWiFiConnected(void) const
{
	wl_status_t status = WiFi.status();
	return (status == WL_CONNECTED);
}

void NetworkManager::addNotificationController(int controllerID)
{
//	DEBUG_PRINT("NetworkManager::addNotificationController: %d\n", controllerID);
	NotificationStruct *ns = new NotificationStruct;
	ns->controllerID = controllerID;
	ns->next = NULL;
	
	if (m_firstNotification == NULL)
	{
		m_firstNotification = ns;
	}
	else
	{
		NotificationStruct *current = m_firstNotification;
		while (current)
		{
			if (current->next != NULL)
			{
				current = current->next;
			}
			else
			{
				current->next = ns;
				break;
			}
		}
	}
}

void NetworkManager::getAddress(int controllerID)
{
	DEBUG_PRINT("NetworkManager::getAddress: %d\n", controllerID);
	UDPMessage message;
	message.setMessageID(SYS_FIND_CONTROLLER);
	message.setID(controllerID);

	sendUdpBroadcastMessage(message);
}

void NetworkManager::checkNotificationList(void)
{
	if (m_currentStruct == NULL)
		m_currentStruct = m_firstNotification;
	if (m_currentStruct)
	{
		unsigned long t = millis();
		if (t - m_lastCheckTimeout > 15000)
		{
			if (m_currentStruct->address[0] == 0)
			{
				getAddress(m_currentStruct->controllerID);
				m_currentStruct = m_currentStruct->next;
			}
			m_lastCheckTimeout = t;
		}
	}
}

bool NetworkManager::updateNotificationList(int controllerID, const IPAddress &address)
{
	bool updated = false;
	NotificationStruct *n = m_firstNotification;

	while (n)
	{
		if (n->controllerID == controllerID)
		{
			if (n->address != address)
			{
				updated = true;
				n->address = address;
			}
			n = NULL;
		}
		else
		{
			n = n->next;
		}
	}
	return updated;
}

void NetworkManager::clearNotificationList(void)
{
	NotificationStruct *n;
	while (m_firstNotification)
	{
		n = m_firstNotification;
		m_firstNotification = n->next;
		delete n;
	}
}

void NetworkManager::setNotificationList(const String &jsonText)
{
	DEBUG_PRINT("NetworkManager::setNotificationList:\n%s\n", jsonText.c_str());
	clearNotificationList();
	StaticJsonBuffer<256> jsonBuffer;
	JsonArray &json = jsonBuffer.parseArray(jsonText);

	for (byte x = 0; x < json.size(); x++)
	{
		unsigned long id = json[x]["id"];
		if (id != m_controllerID)
			addNotificationController(id);
	}

	if (m_notificationListChangedCallback)
		m_notificationListChangedCallback(json);
}

//-----------------------------------------------------------------------------------------------------------------------------//

NetworkManager NetManager;

