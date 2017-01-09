// 
// 
// 

#include "ConfigDownload.h"
#include "Controller.h"
#include "configstructures.h"

ConfigDownloadClass *ConfigDownloadClass::m_this = NULL;

ConfigDownloadClass::ConfigDownloadClass(void)
{
	m_controller = NULL;
	reset();
}

void ConfigDownloadClass::init(Controller *controller)
{
	m_controller = controller;
	m_this = this;
}

void ConfigDownloadClass::process(void)
{
	m_webSocket.loop();
	if (m_serverNotSet && m_buffer != NULL)
	{
		// retry download
		downloadConfig(m_buffer, m_bufferSize, m_key);
	}
}

void ConfigDownloadClass::downloadConfig(uint8_t *buffer, size_t bufferSize, const String &key)
{
	m_serverNotSet = false;
	m_currentPos = 0;
	m_buffer = buffer;
	m_bufferSize = bufferSize;
	m_key = key;

	Serial.print("downloadConfig.  Buffer Size: ");
	Serial.println(bufferSize);
	if (m_controller->getControllerID() != -1 && m_controller->getServerPort() > 0)
	{
		Serial.print("downloadConfig on: ");
		Serial.print(m_controller->getServerAddress().toString());
		Serial.print(":");
		Serial.println(m_controller->getServerPort());

		m_webSocket.begin(m_controller->getServerAddress().toString().c_str(), m_controller->getServerPort());
		//webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
		m_webSocket.onEvent(ConfigDownloadClass::webSocketEvent);
	}
	else
	{
		Serial.println("Server address/port not set!  Server may be off line.");
		m_serverNotSet = true;
	}
}

void ConfigDownloadClass::webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
	switch (type)
	{
	case WStype_DISCONNECTED:
		Serial.printf("[WSc] Disconnected!\n");
		break;
	case WStype_CONNECTED:
	{
		Serial.printf("[WSc] Connected to url: %s\n", payload);

		// Send message to server when Connected containing the controllerID of this controller and
		// a letter indicating the type of controller:
		// 
		if (m_this->m_bufferSize > 0 && m_this->m_buffer != NULL)
			m_this->m_webSocket.sendTXT(m_this->m_key);
	}
	break;
	case WStype_TEXT:
	{
		Serial.printf("[WSc] get text: %s\n", payload);
		break;
	}
	case WStype_BIN:
		Serial.printf("[WSc] get binary length: %d of %d\n", length, m_this->m_bufferSize);

		if (m_this->m_key.length() > 0)
		{
			memcpy(m_this->m_buffer + m_this->m_currentPos, payload, length);
			m_this->m_currentPos += length;
		}
		
		if (m_this->m_currentPos >= m_this->m_bufferSize)
		{
			Serial.println("DOWNLOAD IS FINISHED!!  DISCONNECTING.");
		}
		break;
	}
}

void ConfigDownloadClass::reset(void)
{
	m_key = "";
	m_webSocket.disconnect();
	m_buffer = NULL;
	m_bufferSize = 0;
	m_currentPos = 0;
}

ConfigDownloadClass ConfigDownload;

