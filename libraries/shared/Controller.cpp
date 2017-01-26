#include "Controller.h"
#include "GlobalDefs.h"
#include "ssid.h"

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

Controller::Controller(int localServerPort)
	: m_controllerID(-1), m_class(ClassUnknown), m_serverPort(0), m_lastDNSCount(0), m_dnsCheckTimeout(0)
{
	m_lastDNSService = "turnout";
}

Controller::~Controller()
{
}

// Be sure Controller::loadConfig() is called first!
// DO NOT call loadConfig from this function!!
void Controller::setup(TMessageHandlerFunction messageCallback, ClassEnum controllerClass)
{
	m_class = controllerClass;
	m_messageCallback = messageCallback;

//	m_wifiManager.setup("GCMRR_");
	m_wifiManager.setupWifi("Belkin", password);
	setupNetwork();

	DEBUG_PRINT("------------------------\n");
	DEBUG_PRINT(" ControllerID: %d \r\n", m_controllerID);
	DEBUG_PRINT("------------------------\n");
	DEBUG_PRINT(" Serial #: %d\r\n", ESP.getChipId());
	DEBUG_PRINT("------------------------\n");
}

void Controller::setupNetwork(void)
{
	if(m_udp.begin(UdpPort))
		DEBUG_PRINT("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), UdpPort);
	else
		DEBUG_PRINT("Error starting UDP!\n");
}

void Controller::process(void)
{
	m_wifiManager.process();
	resendLastMessage();
	int packetSize = m_udp.parsePacket();
	if (packetSize >= sizeof(MessageStruct))
	{
		Message message;
		m_udp.read(message.getRef(), sizeof(MessageStruct));
		processMessage(message);
	}
}

void Controller::processMessage(const Message &message)
{
//	DEBUG_PRINT("NEW MESSAGE! MessageID %d\n", message.getMessageID());
	if (message.getMessageID() == SYS_SET_CONTROLLER_ID && message.getLValue() == ESP.getChipId())
	{
		handleSetControllerIDMessage(message);
	}
	else if (message.getMessageID() == SYS_HEARTBEAT)
	{
		handleServerHeartbeatMessage(message);
		m_messageCallback(message);
	}
	else if (message.getMessageID() == SYS_REBOOT_CONTROLLER && (message.getControllerID() == 0 || message.getControllerID() == getControllerID()))
	{
		Serial.println("RESTART MESSAGE! Controller restarting.");
		ESP.restart();
	}
	else if (message.getMessageID() == SYS_DOWNLOAD_FIRMWARE && (message.getByteValue1() == (byte)getClass() || message.getControllerID() == 0 || message.getControllerID() == getControllerID()))
	{
		downloadFirmwareUpdate();
	}
	else
	{
		m_messageCallback(message);
	}
}

void Controller::downloadFirmwareUpdate(void)
{
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	String updateUrl;
	updateUrl = "http://";
	updateUrl += getServerAddress().toString();
	updateUrl += ":82/firmware?ControllerType=";
	updateUrl += m_class;
	DEBUG_PRINT("Checking for firmware update at: %s\n", updateUrl.c_str());
	DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	ESPhttpUpdate.update(updateUrl);

	if (ESPhttpUpdate.getLastError() != 0)
		Serial.println(ESPhttpUpdate.getLastErrorString());
}

void Controller::sendUdpBroadcastMessage(const Message &message)
{
	//DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	//DEBUG_PRINT("MessageSize: %d  MessageID: %d\n", sizeof(MessageStruct), message.getMessageID());
	//DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

	IPAddress broadcastIp;
	broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();
	m_udp.beginPacket(broadcastIp, UdpPort);
	m_udp.write(message.getRef(), sizeof(MessageStruct));
	m_udp.endPacket();
}

void Controller::sendNetworkMessage(const Message &message, bool sendOnce)
{
	if (sendOnce)
	{
		sendUdpBroadcastMessage(message);
	}
	else
	{
		resetSendMessageCounter(message);
		sendUdpBroadcastMessage(message);
	}
}

void Controller::resetSendMessageCounter(const Message &message)
{
	m_resendMessageTimeout = millis();
	m_lastMessage = message;
	m_resendMessageCount = 3;
}

void Controller::resendLastMessage(void)
{
	long t = millis();
	static bool firstTime = true;
	if ((t - m_resendMessageTimeout) >= 1000 && m_resendMessageCount > 0 && m_lastMessage.isValid())
	{
		m_resendMessageTimeout = t;
		sendUdpBroadcastMessage(m_lastMessage);
		m_resendMessageCount--;
	}
}

void Controller::handleSetControllerIDMessage(const Message &message)
{
	DEBUG_PRINT("NEW ControllerID Message! New ControllerID %d\n", message.getControllerID());
	m_controllerID = message.getControllerID();
	EEPROM.put(CONTROLLER_ID_ADDRESS, m_controllerID);
	EEPROM.commit();
}

void Controller::handleServerHeartbeatMessage(const Message &message)
{
	if (message.getField(0) > 0)
	{
		if (m_serverPort == 0)
			DEBUG_PRINT("handleServerHeartbeatMessage Server Address: %d.%d.%d.%d\n", message.getField(0), message.getField(1), message.getField(2), message.getField(3));

		IPAddress address(message.getField(0), message.getField(1), message.getField(2), message.getField(3));
		m_serverPort = message.getField(4);
		m_serverAddress = address;

		if ((m_controllerID < 0 || m_controllerID == 0) && m_serverPort > 0)
		{
			DEBUG_PRINT("NEW CONTROLLER!  SEND NEW CONTROLLER MESSAG\n");
			Message message;
			message.setControllerID(-1);
			message.setMessageID(SYS_NEW_CONTROLLER);
			message.setMessageClass(ClassSystem);
			message.setLValue(ESP.getChipId());
			message.setByteValue1(m_class);

			sendUdpBroadcastMessage(message);
		}
	}
}

bool Controller::checkEEPROM(byte signature)
{
	// return true if the first byte in memory is set to 'signature'.
	// this means the EEPROM memory has valid configuration data stored
	bool valid = true;
	if (EEPROM.read(0) == signature)
	{
		EEPROM.get(CONTROLLER_ID_ADDRESS, m_controllerID);
		DEBUG_PRINT("CONFIGURATION DATA IS VALID!\n");
		DEBUG_PRINT("----------------------------\n%d\n", m_controllerID);
		DEBUG_PRINT("----------------------------\n");
	}
	else
	{
		DEBUG_PRINT("--------------------------------\n");
		DEBUG_PRINT("CONFIGURATION DATA IS INVALID!!!\n");
		DEBUG_PRINT("--------------------------------\n");
		DEBUG_PRINT("--------------------------------\n");
		valid = false;
		m_controllerID = -1;
		EEPROM.put(CONTROLLER_ID_ADDRESS, m_controllerID);
		EEPROM.write(0, signature);
		EEPROM.commit();
	}

	return valid;
}
//
//void Controller::processLocalServer(void)
//{
//	Message message;
//	// Check if a client has connected
//	if (m_server.hasClient())
//	{
//		WiFiClient client = m_server.available();
//		if (!client)
//		{
//			return;
//		}
//		DEBUG_PRINT("New client\n");
//
//		// Wait for data from client to become available
//		while (client.connected() && client.available() < sizeof(MessageStruct))
//		{
//			delay(1);
//		}
//
//		if (client.available() >= sizeof(MessageStruct))
//			client.read((uint8_t *)message.getRef(), sizeof(MessageStruct));
//		DEBUG_PRINT("Done with client\n");
//	}
//	if (message.isValid())
//		processMessage(message);
//
//}
