#pragma once

#include "GlobalDefs.h"
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
	void addRoute(int routeID, const JsonArray &turnouts);

private:
	void reset(void);
	void loadRoute(int routeID);

	Route m_route;
};


extern RouteMonitor Routes;
