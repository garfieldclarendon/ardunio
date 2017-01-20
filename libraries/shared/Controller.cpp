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
	: m_server(localServerPort), m_controllerID(-1), m_class(ClassUnknown), m_serverPort(0), m_lastDNSCount(0), m_dnsCheckTimeout(0)
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

	setupNetwork();

	Serial.println("------------------------");
	Serial.printf (" ControllerID: %d \r\n", m_controllerID);
	Serial.println("------------------------");
	Serial.printf(" Serial #: %d\r\n", ESP.getChipId());
	Serial.println("------------------------");
}

void Controller::setupNetwork(void)
{
	Serial.printf("Connecting to %s ", ssid);
	
	String name("gcmrr_");
	name += ESP.getChipId();
	WiFi.hostname(name);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	
	while (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		delay(1000);
		Serial.print(".");
	}
	Serial.println(" connected");

	// Set up mDNS responder:
	// - first argument is the domain name, 
	//   the fully-qualified domain name is "gcmrr_<chip ID>.local"
	//   where <chip ID> is the id of this ESP8266 (returned by getChipId())
	// - second argument is the IP address to advertise
	//   we send our IP address on the WiFi network

	if (MDNS.begin(name.c_str())) 
		Serial.println("mDNS responder started.  Name: " + name);
	else
		Serial.println("Error setting up MDNS responder!");

	if(m_udp.begin(UdpPort))
		Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), UdpPort);
	else
		Serial.println("Error starting UDP!");

	// Start TCP server
	m_server.begin();
	Serial.println("TCP server started");
}

void Controller::process(void)
{
	MDNS.update();
	processLocalServer();

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
//	Serial.printf("NEW MESSAGE! MessageID %d\n", message.getMessageID());
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
	Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	String updateUrl;
	updateUrl = "http://";
	updateUrl += getServerAddress().toString();
	updateUrl += ":82/firmware?ControllerType=";
	updateUrl += m_class;
	Serial.print("Checking for firmware update at: ");
	Serial.println(updateUrl);
	Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	ESPhttpUpdate.update(updateUrl);

	if (ESPhttpUpdate.getLastError() != 0)
		Serial.println(ESPhttpUpdate.getLastErrorString());
}

void Controller::sendUdpBroadcastMessage(const Message &message)
{
	//Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	//Serial.print("MessageSize: ");
	//Serial.println(sizeof(MessageStruct));
	//Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++");

	IPAddress broadcastIp;
	broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();
	m_udp.beginPacket(broadcastIp, UdpPort);
	m_udp.write(message.getRef(), sizeof(MessageStruct));
	m_udp.endPacket();
}

void Controller::sendNetworkMessage(const Message &message, int deviceID)
{
	String deviceName("device");
	deviceName += deviceID; 
	int n = MDNS.queryService(deviceName, "tcp"); // Send out query for a specific device
	if (n > 0)
	{
		WiFiClient client;

		for (int count = 0; count < n; count++)
		{
			Serial.print("Connecting to: ");
			Serial.println(MDNS.hostname(count));
			if (client.connect(MDNS.IP(count), LocalServerPort))
			{
				Serial.println("Sending message");
				client.write(message.getRef(), sizeof(MessageStruct));
			}
			else
			{
				Serial.print("connection failed.  Failed to connect to: ");
				Serial.println(MDNS.hostname(count));
			}
		}
	}
	else
	{
		Serial.println("query for turnout services returned 0");
	}
}

void Controller::sendNetworkMessage(const Message &message, const String &service)
{
	//Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	//Serial.print("MessageSize: ");
	//Serial.println(sizeof(MessageStruct));
	Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	if (m_lastDNSService != service)
	{
		m_lastDNSService = service;
	}
	updateDNSQuery();

	if (m_lastDNSCount > 0)
	{
		for (int count = 0; count < m_lastDNSCount; count++)
		{
			Serial.print("Connecting to: ");
			Serial.println(MDNS.hostname(count));
			
			WiFiClient client;
			client.setNoDelay(true);
			if (client.connect(MDNS.IP(count), LocalServerPort))
			{
//				Serial.println("Sending message");
				client.write(message.getRef(), sizeof(MessageStruct));
			}
			else
			{
				Serial.print("connection failed.  Failed to connect to: ");
				Serial.println(MDNS.hostname(count));
			}
		}
	}
	else
	{
		Serial.println("query for services returned 0 for: " + service);
	}

	if (m_serverPort > 0)
	{
		WiFiClient client;

		client.setNoDelay(true);
		if (client.connect(m_serverAddress, m_serverPort + 2))
		{
			Serial.println("Sending message to server");
			client.write(message.getRef(), sizeof(MessageStruct));
		}
		else
		{
			m_serverPort = 0;
			Serial.println("Server connection failed");
		}
	}
//	Serial.println("Finished Sending message");
}

void Controller::handleSetControllerIDMessage(const Message &message)
{
	Serial.printf("NEW ControllerID Message! New ControllerID %d\n", message.getControllerID());
	m_controllerID = message.getControllerID();
	EEPROM.put(CONTROLLER_ID_ADDRESS, m_controllerID);
	EEPROM.commit();
}

void Controller::handleServerHeartbeatMessage(const Message &message)
{
	if (message.getField(0) > 0)
	{
		if (m_serverPort == 0)
			Serial.printf("handleServerHeartbeatMessage Server Address: %d.%d.%d.%d\n", message.getField(0), message.getField(1), message.getField(2), message.getField(3));

		IPAddress address(message.getField(0), message.getField(1), message.getField(2), message.getField(3));
		m_serverPort = message.getField(4);
		m_serverAddress = address;

		if ((m_controllerID < 0 || m_controllerID == 0) && m_serverPort > 0)
		{
			Serial.println("NEW CONTROLLER!  SEND NEW CONTROLLER MESSAGE");
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
		Serial.println("-----------------------------");
		Serial.println("-----------------------------");
		Serial.println(m_controllerID);
		Serial.println("-----------------------------");
	}
	else
	{
		Serial.println("-----------------------------");
		Serial.println("CONFIGURATION DATA IS INVALID!!!");
		Serial.println("-----------------------------");
		Serial.println("-----------------------------");
		valid = false;
		m_controllerID = -1;
		EEPROM.put(CONTROLLER_ID_ADDRESS, m_controllerID);
		EEPROM.write(0, signature);
		EEPROM.commit();
	}

	return valid;
}

void Controller::processLocalServer(void)
{
	Message message;
	// Check if a client has connected
	if (m_server.hasClient())
	{
		WiFiClient client = m_server.available();
		if (!client)
		{
			return;
		}
		Serial.println("New client");

		// Wait for data from client to become available
		while (client.connected() && client.available() < sizeof(MessageStruct))
		{
			delay(1);
		}

		if (client.available() >= sizeof(MessageStruct))
			client.read((uint8_t *)message.getRef(), sizeof(MessageStruct));
		Serial.println("Done with client");
	}
	if (message.isValid())
		processMessage(message);

}

void Controller::updateDNSQuery(void)
{
	long t = millis();
	static bool firstTime = true;
	if (firstTime || ((t - m_dnsCheckTimeout) > 10000 && m_lastDNSService.length() > 0))
	{
		Serial.println("updateDNSQuery");
		m_dnsCheckTimeout = t;
		m_lastDNSCount = MDNS.queryService(m_lastDNSService, "tcp");
		firstTime = false;
	}
}
