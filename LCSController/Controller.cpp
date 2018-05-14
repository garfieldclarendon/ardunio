#include "Controller.h"
#include "GlobalDefs.h"
#include "NetworkManager.h"
#include "Local.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <FS.h>

#define blinkingTimeout 200

Controller::Controller(int localServerPort)
	: m_class(ControllerUnknown), m_controllerID(-1)
{
}

Controller::~Controller()
{
}

// Be sure Controller::loadConfig() is called first!
// DO NOT call loadConfig from this function!!
void Controller::setup(ControllerClassEnum controllerClass, int controllerID)
{
	m_class = controllerClass;
	m_controllerID = controllerID;

	DEBUG_PRINT("------------------------\n");
	DEBUG_PRINT(" Serial #:     %d\r\n", ESP.getChipId());
	DEBUG_PRINT(" ControllerID: %d\r\n", m_controllerID);
	DEBUG_PRINT("------------------------\n");
}

void Controller::process(void)
{
}

void Controller::processMessage(const UDPMessage &message)
{
//	DEBUG_PRINT("NEW MESSAGE! MessageID %d  ID %d\n", message.getMessageID(), message.getID());
	if (message.getMessageID() == SYS_REBOOT_CONTROLLER && (message.getID() == 0 || message.getID() == ESP.getChipId()))
	{
		DEBUG_PRINT("REBOOT MESSAGE! Controller restarting.\n");
		restart();
	}
	else if (message.getMessageID() == SYS_DOWNLOAD_FIRMWARE && (message.getID() == 0 || message.getID() == ESP.getChipId()))
	{
		downloadFirmwareUpdate();
	}
	else if (message.getMessageID() == SYS_RESET_CONFIG && (message.getID() == 0 || message.getID() == ESP.getChipId()))
	{
		DEBUG_PRINT("RESET CONTROLLER CONFIG MESSAGE!\n");
		resetConfiguration();
		restart();
	}
	else if (message.getMessageID() == SYS_RESET_NOTIFICATION_LIST && (message.getID() == 0 || message.getID() == ESP.getChipId()))
	{
		DEBUG_PRINT("RESET NOTIFICATION LIST!\n");
		String json = NetManager.getNotificationList();
		NetManager.setNotificationList(json);
	}
	else if (message.getMessageID() == SYS_SERVER_SHUTDOWN)
	{
		DEBUG_PRINT("SERVER SHUTDOWN MESSAGE!\n");
		IPAddress address;
		NetManager.setServerAddress(address);
	}
	else if (message.getMessageID() == SYS_CONTROLLER_ONLINE)
	{
		DEBUG_PRINT("CONTROLLER ONLINE MESSAGE!  CONTROLLER %d\n", message.getID());
		IPAddress address(message.getField(0), message.getField(1), message.getField(2), message.getField(3));
		if (address != (uint32_t)0 && NetManager.updateNotificationList(message.getID(), address))
		{
			if (m_sendStatusCallback)
				m_sendStatusCallback();
		}
	}
	else if (message.getMessageID() == SYS_FIND_CONTROLLER && message.getID() == m_controllerID)
	{
		DEBUG_PRINT("FIND CONTROLLER MESSAGE.  LOOKING FOR ME!\n");
		IPAddress to(message.getField(0), message.getField(1), message.getField(2), message.getField(3));
		DEBUG_PRINT("FIND CONTROLLER MESSAGE.  LOOKING FOR ME!  SENDING MESSAGE TO: %s\n", to.toString().c_str());
		sendControllerOnlineMessage(to);
		if (m_sendStatusCallback)
			m_sendStatusCallback();
	}
	else if (message.getMessageID() == SYS_SERVER_HEARTBEAT)
	{
		IPAddress address(message.getField(0), message.getField(1), message.getField(2), message.getField(3));
//		DEBUG_PRINT("SYS_SERVER_HEARTBEAT: %s\n", address.toString().c_str());
		// A 1 in field 4 indicates the server is just coming online
		if (address != NetManager.getServerAddress() || message.getField(4) == 1)
		{
			DEBUG_PRINT("SYS_SERVER_HEARTBEAT: %s  SAVING ADDRESS\n", address.toString().c_str());
			NetManager.setServerAddress(address);
			if (m_serverFoundCallback)
				m_serverFoundCallback();
			sendControllerOnlineMessage(address);
		}
	}
}

void Controller::downloadFirmwareUpdate(void)
{
	IPAddress address(NetManager.getServerAddress());
	int port(NetManager.getServerPort());

	if (WiFi.status() == WL_CONNECTED && port > 0)
	{
		// ControllerStatusFirmwareUpdate
		UDPMessage message;
		message.setMessageID(SYS_FIRMWARE_UPDATING);
		message.setID(ESP.getChipId());

		NetManager.sendUdpBroadcastMessage(message);
		delay(250);
		DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		String updateUrl("/controller/firmware?ControllerType=");
		updateUrl += m_class;
		DEBUG_PRINT("Checking for firmware update at: %s\n", updateUrl.c_str());
		DEBUG_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		ESPhttpUpdate.update(address.toString(), port, updateUrl);

		if (ESPhttpUpdate.getLastError() != 0)
		{
			DEBUG_PRINT("%s\n", ESPhttpUpdate.getLastErrorString().c_str());
			restart();
		}
	}
}

void Controller::clearFiles(void)
{
	DEBUG_PRINT("CLEAR FILES!\n");
	Dir dir = SPIFFS.openDir("");
	while (dir.next())
	{
		DEBUG_PRINT("DELETING FILE %s\n", dir.fileName().c_str());
		SPIFFS.remove(dir.fileName());
	}

	Dir dir2 = SPIFFS.openDir("\\");
	while (dir2.next())
	{
		DEBUG_PRINT("DELETING FILE %s\n", dir2.fileName().c_str());
		SPIFFS.remove(dir2.fileName());
	}
}

void Controller::resetConfiguration(void)
{
	DEBUG_PRINT("Reset Configuration!\n");
	clearFiles();
}

void Controller::restart(void)
{
	UDPMessage message;
	message.setMessageID(SYS_RESTARTING);
	message.setID(ESP.getChipId());

	NetManager.sendUdpBroadcastMessage(message);
	delay(250);
	ESP.reset();
}

void Controller::networkOnline(void)
{
	DEBUG_PRINT("networkOnline\n");
	IPAddress address;
	sendControllerOnlineMessage(address);
}

void Controller::sendControllerOnlineMessage(IPAddress &address)
{
	IPAddress ip;
	ip = WiFi.localIP();
	UDPMessage message;
	message.setMessageID(SYS_CONTROLLER_ONLINE);
	message.setID(m_controllerID);
	message.setField(0, ip[0]);
	message.setField(1, ip[1]);
	message.setField(2, ip[2]);
	message.setField(3, ip[3]);
	message.setField(5, MajorVersion);
	message.setField(6, MinorVersion);
	message.setField(7, BuildVersion);

	if (address == (uint32_t)0)
		NetManager.sendUdpBroadcastMessage(message);
	else
		NetManager.sendUdpMessage(message, address, true);
}

void Controller::networkOffline(void)
{
}
