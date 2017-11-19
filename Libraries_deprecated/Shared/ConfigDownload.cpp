// 
// 
// 

#include "ConfigDownload.h"
#include "Controller.h"
#include "configstructures.h"
#include <ESP8266mDNS.h>

ConfigDownloadClass *ConfigDownloadClass::m_this = NULL;

ConfigDownloadClass::ConfigDownloadClass(void)
{
	m_controller = NULL;
	m_key = "";
	m_downloadBuffer = "";
}

void ConfigDownloadClass::init(Controller *controller)
{
	m_controller = controller;
	m_this = this;
	m_webSocket.onEvent(ConfigDownloadClass::webSocketEvent);
}

void ConfigDownloadClass::process(void)
{
	m_webSocket.loop();
	if (m_serverNotSet && m_key.length() > 0)
	{
		// retry download
		downloadConfig(m_key, m_configCallback);
	}
}

void ConfigDownloadClass::downloadConfig(const String &key, TConfigCallback callback)
{
	reset();
	m_configCallback = callback;
	m_serverNotSet = false;
	m_key = key;

	IPAddress address;
	int port = -1;
	if (WiFi.status() == WL_CONNECTED)
		getServerAddress(address, port);
	DEBUG_PRINT("downloadConfig.  Key: %s\n", key.c_str());
	if (port != -1 && port > 0)
	{
		DEBUG_PRINT("downloadConfig on: %s:%d\n", address.toString().c_str(), port);

		m_webSocket.begin(address.toString().c_str(), port);
		//webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
	}
	else
	{
		DEBUG_PRINT("Server address/port not set!  Server may be off line.\n");
		m_serverNotSet = true;
	}
}

void ConfigDownloadClass::getServerAddress(IPAddress &address, int &port)
{
	DEBUG_PRINT("Sending mDNS query\n");
	int n = MDNS.queryService("gcmrr-config", "tcp"); // Send out query for GCMRR-CONFIG tcp services
	DEBUG_PRINT("mDNS query done\n");
	if (n == 0)
	{
		DEBUG_PRINT("no services found\n");
	}
	else
	{
		DEBUG_PRINT("%d service(s) found\n", n);
		address = MDNS.IP(0);
		port = MDNS.port(0);
	}
}

void ConfigDownloadClass::webSocketEvent(WStype_t type, uint8_t * payload, size_t length)
{
	switch (type)
	{
	case WStype_DISCONNECTED:
//		DEBUG_PRINT("[WSc] Disconnected!\n");
		break;
	case WStype_CONNECTED:
	{
		DEBUG_PRINT("[WSc] Connected to url: %s  key is: %s\n", payload, m_this->m_key.c_str());

		// Send message to server when Connected containing the key of the desired config data
		// 
		if (m_this->m_key.length() > 0)
			m_this->m_webSocket.sendTXT(m_this->m_key);
	}
	break;
	case WStype_TEXT:
	{
		DEBUG_PRINT("[WSc] get text: %s\n", payload);
		m_this->m_downloadBuffer += (char *)payload;
		if (m_this->m_downloadBuffer.endsWith("\n"))
		{
			m_this->m_key = "";
			m_this->parsePayload();
			DEBUG_PRINT("DOWNLOAD IS FINISHED!!  DISCONNECTING.\n");
		}
		break;
	}
	case WStype_BIN:
		DEBUG_PRINT("[WSc] get binary length: %d\n", length);
		break;
	}
}

// The following function assumes everything is formatted properly for performance purposes.  The text should be:
// <key>,<value>;<key>,<value>;....<key>,<value>;\n
// '\n' denotes the end of the config data
void ConfigDownloadClass::parsePayload(void)
{
	byte x = 0;
	String key;
	String value;
	byte length = m_downloadBuffer.length();
	char sep = ',';

	for (byte x = 0; x < length; x++)
	{
		if (m_downloadBuffer[x] == '\n')
		{
			// All done!
			m_configCallback(NULL, NULL);
		}
		if (m_downloadBuffer[x] != sep)
		{
			if (sep == ',')
				key += m_downloadBuffer[x];
			else
				value += m_downloadBuffer[x];
		}
		else
		{
			if (sep == ',')
			{
				sep = ';';
			}
			else
			{
				DEBUG_PRINT("KEY: %s  VALUE %s\n", key.c_str(), value.c_str());
				m_configCallback(key.c_str(), value.c_str());
				key = "";
				value = "";
				sep = ',';
			}
		}
	}
}

void ConfigDownloadClass::reset(void)
{
	m_key = "";
	m_downloadBuffer = "";
	m_webSocket.disconnect();
}

ConfigDownloadClass ConfigDownload;

