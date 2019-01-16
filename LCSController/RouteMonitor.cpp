#include <FS.h>

#include "RouteMonitor.h"
#include "DeviceMonitor.h"

RouteMonitor::RouteMonitor()
{
}

RouteMonitor::~RouteMonitor()
{
}

void RouteMonitor::reset(void)
{
	memset(&m_route, 0, sizeof(Route));
}

void RouteMonitor::addRoute(int routeID, const JsonArray &turnouts)
{
	reset();
	for (byte x = 0; x < turnouts.size(); x++)
	{
		JsonObject &obj = turnouts[x].asObject();
		m_route.m_entries[x].m_turnoutID = obj["deviceID"];
		m_route.m_entries[x].m_turnoutState = obj["turnoutState"];
	}
	String fileName("/Route_");
	fileName += routeID;
	fileName += ".dat";
	DEBUG_PRINT("SAVE PanelOutputDevice Config %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "w");

	if (f)
	{
		f.write((uint8_t*)&m_route, sizeof(Route));
		f.close();
	}
	else
	{
		DEBUG_PRINT("Error saving RouteMonitor Config file %s\n", fileName.c_str());
	}
}

void RouteMonitor::loadRoute(int routeID)
{
	reset();

	String fileName("/Route_");
	fileName += routeID;
	fileName += ".dat";
	DEBUG_PRINT("RouteMonitor::loadConfig %s\n", fileName.c_str());

	File f = SPIFFS.open(fileName, "r");

	if (f)
	{
		f.read((uint8_t*)&m_route, sizeof(Route));
		f.close();
	}
	else
	{
		DEBUG_PRINT("RouteMonitor Config file %s is missing or can not be opened\n", fileName.c_str());
	}

}

PinStateEnum RouteMonitor::getRouteState(int routeID)
{
	PinStateEnum ret = PinOn;
	loadRoute(routeID);
	for (byte x = 0; x < MAX_ROUTE_ENTRIES; x++)
	{
		if (m_route.m_entries[x].m_turnoutID > 0)
		{
			byte status = Devices.getDeviceStatus(m_route.m_entries[x].m_turnoutID);
			if (status != m_route.m_entries[x].m_turnoutState)
			{
				if (status == TrnToDiverging || status == TrnToNormal)
				{
					ret = PinFlashing;
				}
				else
				{
					ret = PinOff;
				}
				break;
			}
		}
	}

	return ret;
}

//-----------------------------------------------------------------------------------------------------------------------------//
RouteMonitor Routes;