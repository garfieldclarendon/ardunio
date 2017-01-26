#include "ManageWiFi.h"
#include <ESP8266WiFi.h>
#include <limits.h>

ManageWiFi::ManageWiFi(void)
	: m_reconnected(false)
{

}

ManageWiFi::~ManageWiFi(void)
{

}

void ManageWiFi::setupWifi(const String &prefix, const String &password)
{
	m_prefix = prefix;
	m_password = password;
}

void ManageWiFi::process(void)
{
	m_reconnected = false;
	int8_t scanResult;
	wl_status_t status = WiFi.status();
	if (status == WL_DISCONNECTED || status == WL_NO_SSID_AVAIL || status == WL_IDLE_STATUS || status == WL_CONNECT_FAILED) 
	{
		scanResult = WiFi.scanComplete();
		if (scanResult == WIFI_SCAN_RUNNING) 
		{
			// scan is running
			return;
		}
		else if (scanResult > 0) 
		{
			// scan done analyze
			String ssid;
			int bestNetworkDb = INT_MIN;
			uint8 bestBSSID[6];
			int32_t bestChannel;

			DEBUG_PRINT("[WIFI] scan done\n");
			delay(0);

			if (scanResult <= 0) 
			{
				DEBUG_PRINT("[WIFI] no networks found\n");
			}
			else 
			{
				DEBUG_PRINT("[WIFI] %d networks found\n", scanResult);
				for (int8_t i = 0; i < scanResult; ++i) 
				{
					String ssid_scan;
					int32_t rssi_scan;
					uint8_t sec_scan;
					uint8_t* BSSID_scan;
					int32_t chan_scan;
					bool hidden_scan;

					WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);

					bool known = false;
					if (ssid_scan.startsWith(m_prefix) && !ssid_scan.endsWith("guest")) 
					{ // SSID match
						known = true;
						if (rssi_scan > bestNetworkDb) 
						{ // best network
							bestNetworkDb = rssi_scan;
							bestChannel = chan_scan;
							ssid = ssid_scan;
							memcpy((void*)&bestBSSID, (void*)BSSID_scan, sizeof(bestBSSID));
						}
						break;
					}

					if (known) 
					{
						DEBUG_PRINT(" ---> ");
					}
					else 
					{
						DEBUG_PRINT("      ");
					}

					DEBUG_PRINT(" %d: [%d][%02X:%02X:%02X:%02X:%02X:%02X] %s (%d) %c\n", i, chan_scan, BSSID_scan[0], BSSID_scan[1], BSSID_scan[2], BSSID_scan[3], BSSID_scan[4], BSSID_scan[5], ssid_scan.c_str(), rssi_scan, (sec_scan == ENC_TYPE_NONE) ? ' ' : '*');
					delay(0);
				}
			}

			// clean up ram
			WiFi.scanDelete();

			DEBUG_PRINT("\n\n");
			delay(0);

			if (ssid.length() > 0) 
			{
				DEBUG_PRINT("[WIFI] Connecting BSSID: %02X:%02X:%02X:%02X:%02X:%02X SSID: %s Channal: %d (%d)\n", bestBSSID[0], bestBSSID[1], bestBSSID[2], bestBSSID[3], bestBSSID[4], bestBSSID[5], ssid.c_str(), bestChannel, bestNetworkDb);

				WiFi.begin(ssid.c_str(), m_password.c_str(), bestChannel, bestBSSID);
				status = WiFi.status();

				// wait for connection or fail
				while (status != WL_CONNECTED && status != WL_NO_SSID_AVAIL && status != WL_CONNECT_FAILED) 
				{
					delay(10);
					status = WiFi.status();
				}
				if (status == WL_CONNECTED)
					m_reconnected = true;
#ifdef PROJECT_DEBUG
				IPAddress ip;
				uint8_t * mac;
				switch (status) 
				{
				case WL_CONNECTED:
					ip = WiFi.localIP();
					mac = WiFi.BSSID();
					DEBUG_PRINT("[WIFI] Connecting done.\n");
					DEBUG_PRINT("[WIFI] SSID: %s\n", WiFi.SSID().c_str());
					DEBUG_PRINT("[WIFI] IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
					DEBUG_PRINT("[WIFI] MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
					DEBUG_PRINT("[WIFI] Channel: %d\n", WiFi.channel());
					break;
				case WL_NO_SSID_AVAIL:
					DEBUG_PRINT("[WIFI] Connecting Failed AP not found.\n");
					break;
				case WL_CONNECT_FAILED:
					DEBUG_PRINT("[WIFI] Connecting Failed.\n");
					break;
				default:
					DEBUG_PRINT("[WIFI] Connecting Failed (%d).\n", status);
					break;
				}
#endif
			}
			else 
			{
				DEBUG_PRINT("[WIFI] no matching wifi found!\n");
			}
		}
		else 
		{
			// start scan
			DEBUG_PRINT("[WIFI] delete old wifi config...\n");
			WiFi.disconnect();

			DEBUG_PRINT("[WIFI] start scan\n");
			// scan wifi async mode
			WiFi.scanNetworks(true);
		}
	}
}