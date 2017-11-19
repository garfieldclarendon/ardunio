#include <ESP8266HTTPClient.h>

#include "Network.h"
#include "GlobalDefs.h"

NetworkClass *NetworkClass::m_this = NULL;
#define disconnectedTimeout 60000

NetworkClass::NetworkClass(void)
	: m_serverPort(-1), m_serverAddress(0, 0, 0, 0), m_isConnected(false), m_disconnectTimeout(0)
{
	m_this = this;
}

NetworkClass::~NetworkClass(void)
{
}

void NetworkClass::setServerAddress(const IPAddress &value)
{ 
	if (m_serverAddress != value)
	{
		m_serverAddress = value;
		m_webSocket.disconnect();
		m_webSocket.begin(m_serverAddress.toString().c_str(), UdpPort + 1);
		m_webSocket.onEvent(NetworkClass::webSocketEvent);
		DEBUG_PRINT("Connecting to server at %s, Port %d\n", m_serverAddress.toString().c_str(), 8081);
	}
}

void NetworkClass::init(int serverPort)
{
	m_serverPort = serverPort;
	if (m_udp.begin(UdpPort))
		DEBUG_PRINT("Now listening on %s, UDP port %d\n", WiFi.localIP().toString().c_str(), UdpPort);
	else
		DEBUG_PRINT("Error starting UDP!\n");
}

void NetworkClass::process(void)
{
	m_webSocket.loop();
	processUDP();
}

void NetworkClass::processUDP(void)
{
	DEBUG_PRINT("processUDP!  From: %s\n", m_udp.remoteIP().toString().c_str());
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
		DEBUG_PRINT("READING MESSAGE!\n");
		Message message;
		byte size = sizeof(MessageStruct);
		char *ref = message.getRef();
		// Skip the signature bytes
		ref += 2;
		while (m_udp.available() > 0)
		{
			byte b = m_udp.read();
			if (size <= sizeof(MessageStruct))
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
				DEBUG_PRINT("END Signature found!!\n");
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

void NetworkClass::sendUdpBroadcastMessage(const Message &message)
{
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	DEBUG_PRINT("MessageSize: %d  MessageID: %d\n", sizeof(MessageStruct), message.getMessageID());

	IPAddress broadcastIp;
	broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();
	if (m_udp.beginPacket(broadcastIp, UdpPort) == 0)
	{
		DEBUG_PRINT("sendUdpBroadcastMessage beginPacket failed!!!\n");
	}
	else
	{
		m_udp.write(message.getRef(), sizeof(MessageStruct));
		if (m_udp.endPacket() == 0)
		{
			DEBUG_PRINT("sendUdpBroadcastMessage beginPacket failed!!!\n");
		}
	}
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}
void NetworkClass::sendMessageToServer(const JsonObject &json)
{
	String jsonText;
	json.printTo(jsonText);

	DEBUG_PRINT("sendMessageToServer: %s\n", jsonText.c_str());
	m_this->m_webSocket.sendTXT(jsonText);
}

void NetworkClass::webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
	DEBUG_PRINT("[WSc] webSocketEvent: %d\n", type);
	switch (type)
	{
	case WStype_DISCONNECTED:
		DEBUG_PRINT("[WSc] Disconnected!\n");
		if (m_this->m_isConnected)
		{
			m_this->m_isConnected = false;
			m_this->m_disconnectTimeout = millis();
		}
		else
		{
			unsigned long t = millis();
			// If disconnected for more than 60 seconds, reboot just in case something is wrong
			if (t - m_this->m_disconnectTimeout > disconnectedTimeout)
			{
				Message message;
				message.setMessageID(SYS_RESTARTING);
				message.setSerialNumber(ESP.getChipId());

				Network.sendUdpBroadcastMessage(message);
				delay(250);
				ESP.restart();
			}
		}
		break;
	case WStype_CONNECTED:
	{
		DEBUG_PRINT("[WSc] Connected to server: %s\n", payload);
		DEBUG_PRINT("CONNECTED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**************************************************************************************\n", payload);

		m_this->m_isConnected = true;
		m_this->m_disconnectTimeout = 0;
//		 Send message to server when Connected containing the Serial Number
		String json;
        StaticJsonBuffer<250> jsonBuffer;
		JsonObject &root = jsonBuffer.createObject();
		root["messageUri"] = "/controller/connect";
		root["serialNumber"] = ESP.getChipId();
		root["version"] = ControllerVersion;

		root.printTo(json);

		m_this->m_webSocket.sendTXT(json);
		if (m_this->m_serverConnectedCallback)
			m_this->m_serverConnectedCallback();
	}
	break;
	case WStype_TEXT:
	{
		String txt((char *)payload);
		DEBUG_PRINT("raw text: %s\n", txt.c_str());
		StaticJsonBuffer<1024> jsonBuffer;
		JsonObject &root = jsonBuffer.parseObject(txt);

		// return an ACK message
		int transactionID = root["transactionID"];
		String returnString("ACK_");
		returnString += transactionID;
		m_this->m_webSocket.sendTXT(returnString);

		String uri = root["messageUri"];
		NetActionType actionType = (NetActionType)(int)root["action"];
		DEBUG_PRINT("[WStype_TEXT] length: %d, uri: %s Action: %d \n", txt.length(), uri.c_str(), actionType);
		int address = root["address"];
		if (uri == "/controllerConfig")
		{
			if (m_this->m_controllerConfigCallback)
				m_this->m_controllerConfigCallback(actionType, root);
		}
		else if (uri == "/controller/name")
		{
			if (m_this->m_controllerCallback)
				m_this->m_controllerCallback(actionType, root);
		}
		else if (uri == "/controller/module")
		{
			if (m_this->m_moduleCallback)
				m_this->m_moduleCallback(actionType, address, root);
		}
		else if (uri == "/controller/module/config")
		{
			if (m_this->m_moduleConfigCallback)
				m_this->m_moduleConfigCallback(actionType, address, root);
		}
		break;
	}
	case WStype_BIN:
		DEBUG_PRINT("[WSc] get binary length: %d\n", length);
		break;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------//

NetworkClass Network;

