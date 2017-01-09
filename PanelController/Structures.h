// Structures.h

#ifndef _STRUCTURES_h
#define _STRUCTURES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "ConfigStructures.h"

#define MAX_ROUTE_ITEMS 64

class RouteMap
{
public:
	RouteMap(void);
	static void init(void);
	static void addRoute(int routeID);
	static byte getRouteIndex(int routeID);

private:
	static int m_routes[MAX_ROUTE_ITEMS];
};

#endif

