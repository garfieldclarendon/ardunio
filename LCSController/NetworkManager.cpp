#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "NetworkManager.h"
#include "GlobalDefs.h"

NetworkManager *NetworkManager::m_this = NULL;

NetworkManager::NetworkManager(void)
	: m_serverPort(-1), m_controllerID(0), m_serverAddress(0, 0, 0, 0), m_lastCheckTimeout(0), m_firstNotification(NULL), m_currentStruct(NULL), m_wifiStatus(WiFiDisconnected)
{
	m_this = this;
	m_firstMessageQueue = new MessageQueueStruct;
	memset(m_firstMessageQueue, 0, sizeof(MessageQueueStruct));
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
	WiFi.setAutoConnect(false);
	WiFi.setAutoReconnect(true);

	processWiFi();

	if (m_udp.begin(UdpPort))
		DEBUG_PRINT("Now listening UDP port %d\n", UdpPort);
	else
		DEBUG_PRINT("Error starting UDP!\n");
}

bool NetworkManager::process(void)
{
	bool ret = processWiFi();
	processUDP();
	checkNotificationList();
	checkMessageQueue();
	return ret;
}

bool NetworkManager::processWiFi(void)
{
	bool wiFiReconnected = false;
	wl_status_t status = WiFi.status();
	//	DEBUG_PRINT("[WIFI] process::status %d\n", status);
	if (m_wifiStatus == WiFiDisconnected && (status == WL_DISCONNECTED || status == WL_IDLE_STATUS || status == WL_CONNECT_FAILED))
	{
		WiFi.begin(ssid, password);
		status = WiFi.status();
		m_wifiStatus = WiFiConnecting;
		DEBUG_PRINT("[WIFI] Connecting to %s.  Status: %d.\n", ssid, status);
	}
	else if (status == WL_CONNECTED)
	{
		if (m_wifiStatus == WiFiConnecting)
		{
			DEBUG_PRINT("--------------------------------------------------------\n");
			DEBUG_PRINT("[WIFI] CONNECTED TO %s.\n", ssid);
			DEBUG_PRINT("--------------------------------------------------------\n");
			wiFiReconnected = true;
			m_wifiStatus = WiFiConnected;

			if (m_wifiConnectCallback)
				m_wifiConnectCallback(status == WL_CONNECTED);
		}
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
		if (message.getMessageID() == SYS_ACK)
		{
			handleAckMessage(message);
		}
		else
		{
			DEBUG_PRINT("INDEX 0 IS: %d\n", m_udp.destinationIP()[0]);
			if (m_udp.destinationIP()[0] != 255) // Don't send ACK for broadcast messages
			{
				UDPMessage ack;
				ack.setMessageID(SYS_ACK);
				ack.setTransactionNumber(message.getTransactionNumber());
				IPAddress a = m_udp.remoteIP();
				sendUdpMessage(ack, a, false);
			}

			if (m_udpMessageCallback)
				m_udpMessageCallback(message);
		}
	}
}

void NetworkManager::sendUdpBroadcastMessage(const UDPMessage &message)
{
	IPAddress broadcastIp;
	broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();
	sendUdpMessage(message, broadcastIp, false);
}

void NetworkManager::sendUdpMessage(const UDPMessage &message, bool addToQueue)
{
//	DEBUG_PRINT("SENDUDPMESSAGE: MessageID: %d  Transaction: %d\n",message.getMessageID(), message.getTransactionNumber());
	NotificationStruct *current = m_firstNotification;

	while (current)
	{
		if (current->address != (uint32_t)0)
		{
			if (sendUdpMessage(message, current->address, addToQueue) == false)
				current->address = IPAddress();
		}
		else
		{
			DEBUG_PRINT("Missing address for controller: %d\n", current->controllerID);
		}
		current = current->next;
	}

	if (m_serverAddress != (uint32_t)0)
	{
		sendUdpMessage(message, m_serverAddress, true);
	}

	// Give other modules connected to this controller a chance to process the message too
	if (m_udpMessageCallback)
		m_udpMessageCallback(message);
}

bool NetworkManager::sendUdpMessage(const UDPMessage &message, IPAddress &address, bool addToQueue)
{
//	DEBUG_PRINT("SENDUDPMESSAGE WITH ADDRESS: %s  Transaction: %d\n", address.toString().c_str(), message.getTransactionNumber());
	UDPMessage newMessage;
	if (addToQueue)
	{
		newMessage.copyFrom(message);
		addMessageToQueue(newMessage, address);
	}
	else
	{
		newMessage = message;
	}
//	DEBUG_PRINT("SENDUDPMESSAGE WITH ADDRESS: %s  NEW Transaction: %d\n", address.toString().c_str(), newMessage.getTransactionNumber());

	bool ret = false;
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	DEBUG_PRINT("MessageSize: %d  MessageID: %d ID: %d TO %s\n", sizeof(UDPMessageStruct), message.getMessageID(), message.getID(), address.toString().c_str());

	if (m_udp.beginPacket(address, UdpPort) == 0)
	{
		DEBUG_PRINT("sendUdpMessage beginPacket failed!!!\n");
	}
	else
	{
		m_udp.write(newMessage.getRef(), sizeof(UDPMessageStruct));
		if (m_udp.endPacket() == 0)
		{
			DEBUG_PRINT("sendUdpMessage beginPacket failed!!!\n");
		}
		else
		{
			ret = true;
		}
	}
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	yield();
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

String NetworkManager::getSignalAspect(int aspectID)
{
	String url("/controller/device/config?aspectID=");
	url += aspectID;
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

	m_udp.stop();
	http.setReuse(false);
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
	DEBUG_PRINT("[HTTP] GET... code: %d\n", httpCode);
	String payload;
	// httpCode will be negative on error
	if (httpCode > 0) {
		// HTTP header has been sent and Server response header has been handled

		// file found at server
		if (httpCode == HTTP_CODE_OK) 
		{
			payload = http.getString();
			DEBUG_PRINT("Size: %d\n%s\n", payload.length(), payload.c_str());
		}
	}
	else 
	{
		DEBUG_PRINT("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();
	yield();
	m_udp.begin(UdpPort);
	return payload;
}

bool NetworkManager::getWiFiConnected(void) const
{
	return m_wifiStatus == WiFiConnected;
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
	IPAddress ip;
	ip = WiFi.localIP();
	message.setField(0, ip[0]);
	message.setField(1, ip[1]);
	message.setField(2, ip[2]);
	message.setField(3, ip[3]);
	message.setField(5, MajorVersion);
	message.setField(6, MinorVersion);
	message.setField(7, BuildVersion);

	sendUdpBroadcastMessage(message);
}

void NetworkManager::checkNotificationList(void)
{
	if (m_currentStruct == NULL)
		m_currentStruct = m_firstNotification;
	if (m_currentStruct)
	{
		unsigned long t = millis();
		if ((t - m_lastCheckTimeout) >= 3000)
		{
			if (m_currentStruct->address == (uint32_t)0)
			{
				getAddress(m_currentStruct->controllerID);
			}
			m_currentStruct = m_currentStruct->next;
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
	StaticJsonBuffer<1024> jsonBuffer;
	JsonArray &json = jsonBuffer.parseArray(jsonText);

	for (byte x = 0; x < json.size(); x++)
	{
		unsigned long id = json[x]["controllerID"];
		if (id != m_controllerID)
			addNotificationController(id);
	}

	if (m_notificationListChangedCallback)
		m_notificationListChangedCallback(json);
}

void NetworkManager::addMessageToQueue(const UDPMessage &message, const IPAddress &address)
{
	DEBUG_PRINT("addMessageToQueue: %d\n", message.getTransactionNumber());
	byte count = 0;
	MessageQueueStruct *current = m_firstMessageQueue;
	bool done = false;

	// first, see if this is an updated version of a message already in the queue.  If so, replace that message and return
	while (current)
	{
		if (current->message.getMessageID() == message.getMessageID() && current->message.getID() == message.getID() && current->address == address)
		{
			DEBUG_PRINT("addMessageToQueue: REPLACING %d with %d\n", message.getTransactionNumber(), message.getTransactionNumber());
			current->message = message;
			current->count = 0;
			return;
		}
		current = current->next;
	}
	current = m_firstMessageQueue;
	while (current)
	{
		if (current->message.getMessageID() == 0)
		{
			current->count = 0;
			current->message = message;
			current->address = address;
			current = NULL;
			done = true;
		}
		else
		{
			count++;
			if (current->next)
				current = current->next;
			else
				break;
		}
	}
	
	if (done == false)
	{
		if (count < 32)
		{
			MessageQueueStruct *hold = current;
			current = new MessageQueueStruct;
			hold->next = current;
		}
		else
		{
			current = m_firstMessageQueue; //OVERFLOW!  Overwrite the oldest (first)
		}

		memset(current, 0, sizeof(MessageQueueStruct));
		current->message = message;
		current->address = address;
	}
}

void NetworkManager::handleAckMessage(const UDPMessage &message)
{
//	DEBUG_PRINT("handleAckMessage: %d  Transaction %d\n", message.getMessageID(), message.getTransactionNumber());
	MessageQueueStruct *current = m_firstMessageQueue;

	while (current)
	{
		if (current->message.getTransactionNumber() == message.getTransactionNumber())
		{
			DEBUG_PRINT("FOUND ENTRY: %d\n", current->message.getTransactionNumber());
			UDPMessage message;
			current->message = message;
			current->count = 0;
			current = NULL;
		}
		else
		{
			current = current->next;
		}
	}
}

void NetworkManager::checkMessageQueue(void)
{
	static unsigned long timeout = 0;

	unsigned long t = millis();
	if ((t - timeout) >= 1000)
	{
		timeout = t;
		MessageQueueStruct *current = m_firstMessageQueue;

		while (current)
		{
			current->count++;
			if (current->message.getMessageID() > 0 && current->count >= 2 && current->count < 6)
			{
				DEBUG_PRINT("MESSAGE RETRY: %d  MessageID %d\n", current->message.getTransactionNumber(), current->message.getMessageID());
				sendUdpMessage(current->message, current->address, false);
			}
			else
			{
				if (current->message.getMessageID() > 0)
				{
					if (current->count >= 6) // give up
					{
						DEBUG_PRINT("GIVING UP ON MESSAGE RETRY: %d\n", current->message.getTransactionNumber());
						UDPMessage message;
						current->message = message;
						current->address = (uint32_t)0;
						current->count = 0;
					}
				}
			}
			current = current->next;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------//

NetworkManager NetManager;

