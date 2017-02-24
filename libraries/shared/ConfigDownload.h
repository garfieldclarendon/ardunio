// ConfigDownload.h

#ifndef _CONFIGDOWNLOAD_h
#define _CONFIGDOWNLOAD_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <WebSocketsClient.h>

class Controller;

class ConfigDownloadClass
{
 protected:


 public:
	 typedef std::function<void(const char *key, const char *value)> TConfigCallback;

	 ConfigDownloadClass(void);
	 void init(Controller *controller);
//	void downloadConfig(uint8_t *buffer, size_t bufferSize, const String &key);
	 void downloadConfig(const String &key, TConfigCallback callback);
	void process(void);
	void reset(void);

private:
	void getServerAddress(IPAddress &address, int &port);
	static ConfigDownloadClass *m_this;
	static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
	void parsePayload(void);
	WebSocketsClient m_webSocket;
	Controller *m_controller;
	String m_key;
	String m_downloadBuffer;
	bool m_serverNotSet;

	TConfigCallback m_configCallback;
};

extern ConfigDownloadClass ConfigDownload;

#endif

