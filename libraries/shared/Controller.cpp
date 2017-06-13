#include "Controller.h"
#include "GlobalDefs.h"
#include "Network.h"

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <FS.h>

#define blinkingTimeout 200

Controller::Controller(int localServerPort)
	: m_controllerID(-1), m_class(ClassUnknown), m_currentBlinkTimeout(0), m_findServerTimeout(0), m_serverFound(false)
{
	memset(&m_devices, 0, sizeof(DeviceEntryStruct) * MAX_MODULES);
	memset(&m_extraPins, 0, 8);
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
	WiFi.setAutoConnect(false);
	WiFi.disconnect();

	m_wifiManager.setupWifi(ssidPrefix, password);

	DEBUG_PRINT("------------------------\n");
	DEBUG_PRINT(" Serial #: %d\r\n", ESP.getChipId());
	DEBUG_PRINT("------------------------\n");
}

void Controller::process(void)
{
	m_wifiManager.process();
	if (m_wifiManager.getIsReconnected())
	{
		Network.init(8080);
		findServer();
		if (m_wifiReconnectCallback)
			m_wifiReconnectCallback();
	}
	else if (m_serverFound == false)
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

void Controller::processMessage(const Message &message)
{
	DEBUG_PRINT("NEW MESSAGE! MessageID %d\n", message.getMessageID());
	if (message.getMessageID() == SYS_REBOOT_CONTROLLER && (message.getSerialNumber() == 0 || message.getSerialNumber() == ESP.getChipId()))
	{
		DEBUG_PRINT("RESTART MESSAGE! Controller restarting.\n");
		restart();
	}
	else if (message.getMessageID() == SYS_DOWNLOAD_FIRMWARE && (message.getSerialNumber() == 0 || message.getSerialNumber() == ESP.getChipId()))
	{
		downloadFirmwareUpdate();
	}
	else if (message.getMessageID() == SYS_RESET_CONFIG && message.getSerialNumber() == ESP.getChipId())
	{
		DEBUG_PRINT("RESET CONFIG MESSAGE!\n");
		resetConfiguration();
		restart();
	}
	else if (message.getMessageID() == SYS_SERVER_HEARTBEAT)
	{
		m_serverFound = true;
		IPAddress address(message.getField(0), message.getField(1), message.getField(2), message.getField(3));
		Network.setServerAddress(address);
		DEBUG_PRINT("SYS_SERVER_HEARTBEAT: %s\n", address.toString().c_str());
		if (m_controllerID <= 0)
			getControllerIDAndName();
	}
}

void Controller::getControllerIDAndName(void)
{
	DEBUG_PRINT("getControllerIDAndName\n");
	StaticJsonBuffer<600> jsonBuffer;
	JsonObject &root = jsonBuffer.createObject();
	root["serialNumber"] = ESP.getChipId();
	root["messageUri"] = "/controller/connect";
	root["version"] = ControllerVersion;

	String json;
	Network.sendMessageToServer(root);
}

void Controller::updateControllerName(const JsonObject &root)
{
	m_controllerID = root["controllerID"];
	m_controllerName = (const char *)root["controllerName"];
	DEBUG_PRINT("getControllerIDAndName: Set controllerID %d and controllerName %s\n", m_controllerID, m_controllerName.c_str());
	JsonArray &devices = root["devices"];
	for (byte x = 0; x < devices.size(); x++)
	{
		m_devices[x].deviceID = devices[x]["deviceID"];
		m_devices[x].deviceName = (const char *)devices[x]["deviceName"];
		DEBUG_PRINT("getControllerIDAndName: DeviceID %d  Name %s\n", m_devices[x].deviceID, m_devices[x].deviceName.c_str());
	}
}

void Controller::controllerCallback(NetActionType actionType, const JsonObject &root)
{
	String uri = root["messageUri"];
	if (uri == "controller/name")
		updateControllerName(root);
}

byte Controller::getDeviceCount(void) const
{
	byte count = 0;

	for (byte x = 0; x < MAX_MODULES; x++)
	{
		if (m_devices[x].deviceID == 0)
			break;
		else
			count++;
	}
	return count;
}

int Controller::getDeviceID(byte index) const
{
	return m_devices[index].deviceID;
}

String Controller::getDeviceName(byte index) const
{
	return m_devices[index].deviceName;
}

void Controller::downloadFirmwareUpdate(void)
{
	IPAddress address(Network.getServerAddress());
	int port(Network.getServerPort());

	if (WiFi.status() == WL_CONNECTED && port > 0)
	{
		DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		String updateUrl;
		updateUrl = "http://";
		updateUrl += address.toString() + ":" + port;
		updateUrl += "/firmware?ControllerType=";
		updateUrl += m_class;
		DEBUG_PRINT("Checking for firmware update at: %s\n", updateUrl.c_str());
		DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		ESPhttpUpdate.update(updateUrl);

		if (ESPhttpUpdate.getLastError() != 0)
			Serial.println(ESPhttpUpdate.getLastErrorString());
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
	Message message;
	message.setMessageID(SYS_RESTARTING);
	message.setSerialNumber(ESP.getChipId());

	Network.sendUdpBroadcastMessage(message);
	delay(250);
	ESP.restart();
}

void Controller::findServer(void)
{
	DEBUG_PRINT("findServer!\n");

	IPAddress ip;
	ip = WiFi.localIP();
	Message message;
	message.setMessageID(SYS_FIND_SERVER);
	message.setSerialNumber(ESP.getChipId());
	message.setField(0, ip[0]);
	message.setField(1, ip[1]);
	message.setField(2, ip[2]);
	message.setField(3, ip[3]);

	Network.sendUdpBroadcastMessage(message);
}

void Controller::addExtraPin(byte virtualPin, byte physicalPin, PinModeEnum mode)
{
	m_extraPins[virtualPin] = physicalPin;

	if (mode == PinInput)
		pinMode(physicalPin, INPUT);
	else if (mode == PinInputPullup)
		pinMode(physicalPin, INPUT_PULLUP);
	else if (mode == PinOutput)
		pinMode(physicalPin, OUTPUT);
	else if (mode == PinOutputOpenDrain)
		pinMode(physicalPin, OUTPUT_OPEN_DRAIN);
}

void Controller::setExtraPin(byte virtualPin, PinStateEnum pinState)
{
	if (pinState == PinFlashing)
	{
		addFlashingPin(m_extraPins[virtualPin]);
	}
	else
	{
		removeFlashingPin(m_extraPins[virtualPin]);
		digitalWrite(m_extraPins[virtualPin], pinState == PinOn ? HIGH : LOW);
	}
}

PinStateEnum Controller::getExtraPin(byte virtualPin)
{
	PinStateEnum ret;
	
	if (digitalRead(m_extraPins[virtualPin]) == HIGH)
		ret = PinOn;
	else
		ret = PinOff;

	return ret;
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
