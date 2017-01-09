// 
// 
// 

#include "Structures.h"


int RouteMap::m_routes[MAX_ROUTE_ITEMS];

RouteMap::RouteMap(void)
{

}

void RouteMap::init(void)
{
	memset(&m_routes, 0, MAX_ROUTE_ITEMS);
}

void RouteMap::addRoute(int routeID)
{
	if (routeID > 0)
	{
		for (byte x = 0; x < MAX_ROUTE_ITEMS; x++)
		{
			if (m_routes[x] == 0)
			{
				m_routes[x] = routeID;
				break;
			}
		}
	}
}

byte RouteMap::getRouteIndex(int routeID)
{
	byte index = 0;
	for (byte x = 0; x < MAX_ROUTE_ITEMS; x++)
	{
		if (m_routes[x] == routeID)
		{
			index = x;
			break;
		}
	}

	return index;
}
