#pragma once

#include "Arduino.h"
#include "GlobalDefs.h"

class ManageWiFi{

public:
	ManageWiFi(void);
	~ManageWiFi(void);

	void setupWifi(const String &prefix, const String &password);
	void process(void);
	bool getIsReconnected(void) const { return m_reconnected;  }

private:
	String m_prefix;
	String m_password;
	bool m_reconnected;
};
