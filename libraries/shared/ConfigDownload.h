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
	 ConfigDownloadClass(void);
	 void init(Controller *controller);
	void downloadConfig(uint8_t *buffer, size_t bufferSize, const String &key);
	void process(void);
	bool downloadComplete(void) const { return m_buffer != NULL && m_currentPos >= m_bufferSize;  }
	void reset(void);

private:
	static ConfigDownloadClass *m_this;
	static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
	WebSocketsClient m_webSocket;
	Controller *m_controller;
	String m_key;
	uint8_t *m_buffer;
	size_t m_bufferSize;
	size_t m_currentPos;
	bool m_serverNotSet;
	long m_lastFirmwareCheckTimeout;
};

extern ConfigDownloadClass ConfigDownload;

#endif

