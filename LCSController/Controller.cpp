#include "Controller.h"
#include "GlobalDefs.h"
#include "NetworkManager.h"
#include "Local.h"

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <FS.h>

#define blinkingTimeout 200

Controller::Controller(int localServerPort)
	: m_class(ClassUnknown), m_currentBlinkTimeout(0), m_findServerTimeout(0), m_serverFound(false)
{
	memset(&m_blinkingPins, 255, 8);
}

Controller::~Controller()
{
}

// Be sure Controller::loadConfig() is called first!
// DO NOT call loadConfig from this function!!
void Controller::setup(ClassEnum controllerClass)
{
	m_class = controllerClass;

	DEBUG_PRINT("------------------------\n");
	DEBUG_PRINT(" Serial #: %d\r\n", ESP.getChipId());
	DEBUG_PRINT("------------------------\n");
}

void Controller::process(void)
{
	if (m_serverFound == false)
	{
		unsigned long t = millis();
		if (t - m_findServerTimeout > 10000)
		{
			m_findServerTimeout = t;
			findServer();
		}
	}
	flashPins();
}

void Controller::processMessage(const UDPMessage &message)
{
	DEBUG_PRINT("NEW MESSAGE! MessageID %d\n", message.getMessageID());
	if (message.getMessageID() == SYS_REBOOT_CONTROLLER && (message.getID() == 0 || message.getID() == ESP.getChipId()))
	{
		DEBUG_PRINT("REBOOT MESSAGE! Controller restarting.\n");
		restart();
	}
	else if (message.getMessageID() == SYS_DOWNLOAD_FIRMWARE && (message.getID() == 0 || message.getID() == ESP.getChipId()))
	{
		downloadFirmwareUpdate();
	}
	else if (message.getMessageID() == SYS_RESET_CONFIG && message.getID() == ESP.getChipId())
	{
		DEBUG_PRINT("RESET CONTROLLER CONFIG MESSAGE!\n");
		resetConfiguration();
		restart();
	}
	else if (message.getMessageID() == SYS_SERVER_HEARTBEAT)
	{
		m_serverFound = true;
		IPAddress address(message.getField(0), message.getField(1), message.getField(2), message.getField(3));
		DEBUG_PRINT("SYS_SERVER_HEARTBEAT: %s\n", address.toString().c_str());
		if (address != NetManager.getServerAddress())
		{
			NetManager.setServerAddress(address);
			if (m_serverFoundCallback)
				m_serverFoundCallback;
		}
	}
}

void Controller::downloadFirmwareUpdate(void)
{
	IPAddress address(NetManager.getServerAddress());
	int port(NetManager.getServerPort());

	if (WiFi.status() == WL_CONNECTED && port > 0)
	{
		DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		String updateUrl;
		updateUrl += "/firmware?ControllerType=";
		updateUrl += m_class;
		DEBUG_PRINT("Checking for firmware update at: %s\n", updateUrl.c_str());
		DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		ESPhttpUpdate.update(address.toString(), port, updateUrl);

		if (ESPhttpUpdate.getLastError() != 0)
		{
			Serial.println(ESPhttpUpdate.getLastErrorString());
			restart();
		}
	}
}

/*
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
		clearFiles();
	}

	return valid;
}
*/
void Controller::clearFiles(void)
{
	DEBUG_PRINT("CLEAR FILES!\n");
	Dir dir = SPIFFS.openDir("");

	while (dir.next())
	{
		DEBUG_PRINT("DELETING FILE %s\n", dir.fileName().c_str());
		SPIFFS.remove(dir.fileName());
	}
}

void Controller::resetConfiguration(void)
{
	DEBUG_PRINT("Reset Configuration!\n");

	byte signature(0);

	EEPROM.write(0, signature);
	EEPROM.commit();
	clearFiles();
}

void Controller::restart(void)
{
	UDPMessage message;
	message.setMessageID(SYS_RESTARTING);
	message.setID(ESP.getChipId());

	NetManager.sendUdpBroadcastMessage(message);
	delay(250);
	ESP.restart();
}

void Controller::findServer(void)
{
	m_serverFound = false;
	DEBUG_PRINT("findServer!\n");

	IPAddress ip;
	ip = WiFi.localIP();
	UDPMessage message;
	message.setMessageID(SYS_FIND_SERVER);
	message.setID(ESP.getChipId());
	message.setField(0, ip[0]);
	message.setField(1, ip[1]);
	message.setField(2, ip[2]);
	message.setField(3, ip[3]);

	NetManager.sendUdpBroadcastMessage(message);
}

void Controller::addFlashingPin(byte pin)
{
	bool found = false;
	// Make sure the pin is not already in the list
	for (byte x = 0; x < 8; x++)
	{
		if (m_blinkingPins[x] == pin)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		// Add the pin to the first empty slot
		for (byte x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] == 255)
			{
				m_blinkingPins[x] = pin;
				break;
			}
		}
	}
}

void Controller::removeFlashingPin(byte pin)
{
	for (byte x = 0; x < 8; x++)
	{
		if (m_blinkingPins[x] == pin)
		{
			m_blinkingPins[x] = 255;
		}
	}
}

void Controller::flashPins(void)
{
	unsigned long t = millis();
	if (t - m_currentBlinkTimeout > blinkingTimeout)
	{
		m_currentBlinkTimeout = t;
		for (int x = 0; x < 8; x++)
		{
			if (m_blinkingPins[x] != 255)
			{
				byte state = digitalRead(m_blinkingPins[x]);
				digitalWrite(m_blinkingPins[x], state == 0);
				DEBUG_PRINT("FLASHING PIN %d\n", m_blinkingPins[x]);
			}
		}
	}
}
