#include <FS.h>

#include "RouteMonitor.h"
#include "DeviceMonitor.h"
#include "NetworkManager.h"


extern StaticJsonBuffer<2048> jsonBuffer;

RouteMonitor::RouteMonitor()
	: m_currentRoute(0)
{
}

RouteMonitor::~RouteMonitor()
{
}

void RouteMonitor::reset(void)
{
	m_currentRoute = 0;
	memset(&m_route, 0, sizeof(Route));
}

void RouteMonitor::removeRoute(int routeID)
{
	String fileName(createFileName(routeID));
	SPIFFS.remove(fileName);
}

void RouteMonitor::addRoute(int routeID)
{
	DEBUG_PRINT("RouteMonitor::addRoute: ADDING ROUTE %d\n", routeID);
	String fileName(createFileName(routeID));

	if (SPIFFS.exists(fileName) == false)
	{
		String jsonText = NetManager.getRouteList(routeID);
		if (jsonText.length() > 0)
		{
			jsonBuffer.clear();
			JsonArray &a = jsonBuffer.parseArray(jsonText);
			addRoute(routeID, a);
		}
	}
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
	String fileName(createFileName(routeID));
	DEBUG_PRINT("SAVE RouteMonitor::addRoute file %s\n", fileName.c_str());

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
	DEBUG_PRINT("SAVE RouteMonitor::addRoute EXITING\n");
}

void RouteMonitor::processMessage(const UDPMessage & message)
{
	if (message.getMessageID() == SYS_ROUTE_CONFIG_CHANGED)
	{
		removeRoute(message.getID());
		addRoute(message.getID());
	}
}

void RouteMonitor::loadRoute(int routeID)
{
	if (routeID != m_currentRoute)
	{
		reset();

		String fileName(createFileName(routeID));
		DEBUG_PRINT("RouteMonitor::loadConfig %s\n", fileName.c_str());

		File f = SPIFFS.open(fileName, "r");

		if (f)
		{
			f.read((uint8_t*)&m_route, sizeof(Route));
			f.close();
			m_currentRoute = routeID;
		}
		else
		{
			DEBUG_PRINT("RouteMonitor Config file %s is missing or can not be opened\n", fileName.c_str());
		}
	}
}

PinStateEnum RouteMonitor::getRouteState(int routeID)
{
	PinStateEnum ret = PinOff;
	loadRoute(routeID);
	if (m_currentRoute == routeID)
	{
		for (byte x = 0; x < MAX_ROUTE_ENTRIES; x++)
		{
			if (m_route.m_entries[x].m_turnoutID > 0)
			{
				byte status = Devices.getDeviceStatus(m_route.m_entries[x].m_turnoutID);
				DEBUG_PRINT("RouteMonitor::getRouteState Turnout: %d  Current State: %d  TargetState: %d\n", m_route.m_entries[x].m_turnoutID, status, m_route.m_entries[x].m_turnoutState);
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
				else
				{
					ret = PinOn;
				}
			}
		}
	}

	return ret;
}

TurnoutState RouteMonitor::getTurnoutStateForRoute(int deviceID, int routeID)
{
	TurnoutState turnoutState(TrnUnknown);
	loadRoute(routeID);
	// Make sure the route was actually loaded
	if (m_currentRoute = routeID)
	{
		for (byte x = 0; x < MAX_ROUTE_ENTRIES; x++)
		{
			if (m_route.m_entries[x].m_turnoutID > 0 && m_route.m_entries[x].m_turnoutID == deviceID)
			{
				turnoutState = (TurnoutState)m_route.m_entries[x].m_turnoutState;
				break;
			}
		}
	}
	return turnoutState;
}

String RouteMonitor::createFileName(int routeID)
{
	String fileName("/Route_");
	fileName += routeID;
	fileName += ".dat";
	return fileName;
}

//-----------------------------------------------------------------------------------------------------------------------------//
RouteMonitor Routes;