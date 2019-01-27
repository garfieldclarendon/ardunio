#pragma once

#include "GlobalDefs.h"
#include "UDPMessage.h"

#include <ArduinoJson.h>

struct RouteEntry
{
	int m_turnoutID;
	byte m_turnoutState;
};
typedef struct RouteEntry RouteEntry;

struct Route
{
	byte m_version;
	RouteEntry m_entries[MAX_ROUTE_ENTRIES];
};
typedef struct Route Route;

class RouteMonitor
{
public:
	RouteMonitor();
	~RouteMonitor();
	PinStateEnum getRouteState(int routeID);
	TurnoutState getTurnoutStateForRoute(int deviceID, int routeID);

	void addRoute(int routeID, const JsonArray &turnouts);
	void addRoute(int routeID);
	void processMessage(const UDPMessage &message);

private:
	void reset(void);
	void loadRoute(int routeID);
	void removeRoute(int routeID);
	String createFileName(int routeID);

	Route m_route;
	int m_currentRoute;
};


extern RouteMonitor Routes;
