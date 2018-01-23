#ifndef APIROUTE_H
#define APIROUTE_H

#include <QObject>
#include <QList>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"

/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation
/**
 * @api {get} /api/activate_route:routeID Activate Route
 * @apiName ActivateRoute
 * @apiGroup Route
 *
 * @apiParam {Number} routeID The id of the route to activate.
 * @apiDescription Activates the specified route.
 * @apiExample Example usage:
 * http://localhost:8080/api/activate_route?routeID=6
 */

/**
 * @api {get} /api/lock_route:routeID,lock Lock Route
 * @apiName LockRoute
 * @apiGroup Route
 *
 * @apiParam {Number} routeID The id of the route to lock.
 * @apiParam {Number=0,1} lock Lock flag.  0 = Unlock, 1 = Lock.
 * @apiDescription Activates and locks the specified route.  Locking a route locks all turnouts in the route's list.  The route must be unlocked in
 * in order to activate any turnouts assigned to this route.
 * @apiExample Example usage:
 * http://localhost:8080/api/lock_route?routeID=6
 */

/**
 * @api {get} /api/route_list: Route List
 * @apiName RouteList
 * @apiGroup Route
 *
 * @apiDescription Returns a list of all routes and the routes current state.
 * @apiSuccess {Number} canLock Flag indicating if the route can be locked.
 * @apiSuccess {Number} isActive Flag indicating if the route is currently active.
 * @apiSuccess {Number} isLocked Flag indicating if the route is currently locked.
 * @apiSuccess {String} routeDescription Route's description.
 * @apiSuccess {Number} routeID Route's ID.
 * @apiSuccess {String} routeName Route's name.
 * @apiExample Example usage:
 * http://localhost:8080/api/route_list
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "canLock": true,
 *              "isActive": false,
 *              "isLocked": false,
 *              "routeDescription": "To Mine",
 *              "routeID": "7",
 *              "routeName": "CA 1"
 *          }, {
 *              "canLock": true,
 *              "isActive": false,
 *              "isLocked": false,
 *              "routeDescription": "West X-Over 2 (Restore)",
 *              "routeID": "16",
 *              "routeName": "CA 10"
 *          }
 *      ]
 */

/**
 * @api {get} /api/route_entry_list:routeID Route Entry List
 * @apiName RouteEntryList
 * @apiGroup Route
 *
 * @apiParam {Number} routeID ID of the route.
 * @apiDescription Returns a list route entries which define the turnouts that are set when the route is activated.
 * @apiSuccess {Number} deviceID deviceID of the turnout.
 * @apiSuccess {Number} routeEntryID routeEntry's ID field.
 * @apiSuccess {Number} routeID routeID of the route the entry is assigned to.
 * @apiSuccess {Number=1,3} turnoutState The state the turnout is to be set to when the route is activated.  1 = Normal, 3 = Diverging (thrown)
 * @apiExample Example usage:
 * http://localhost:8080/api/route_entry_list?routeID=16
 * @apiSuccessExample {json} Success-Response:
 *      HTTP/1.1 200 OK
 *      [{
 *              "deviceID": "15",
 *              "routeEntryID": "23",
 *              "routeID": "16",
 *              "turnoutState": "1"
 *          }, {
 *              "deviceID": "16",
 *              "routeEntryID": "24",
 *              "routeID": "16",
 *              "turnoutState": "1"
 *          }
 *      ]
 */

/**
 * @api {get} /api/notification/route Route Status Change
 * @apiName RouteStatusChangeNotification
 * @apiGroup APINotifications
 *
 * @apiDescription Notification message sent when a route's state changes.
 * @apiSuccess {String} url Notification url.
 * @apiSuccess {Number} routeID Route's ID.
 * @apiSuccess {Number=0,1} isActive Flag indicating if the route is active. 0 = not active, 1 = active.
 * @apiSuccess {Number=0,1} isLocked Flag indicating if the route is currently locked.  0 = not locked, 1 = locked
 * @apiSuccess {Number=0,1} canLock Flag indicating if the lock is allowed to be locked. 0 = cannot be locked, 1 = can be locked.  If
 * any of the turnouts contained in the route are currently in a locked state, the route cannot be locked.
 * @apiSuccessExample {json} Success-Response:
 *      {
 *              "url": "/api/notification/device"
 *              "deviceID": "1"
 *              "deviceState": "2"
 *              "locked": "0"
 *      }
 *
 */

class APIRoute : public QObject
{
    Q_OBJECT
public:
    explicit APIRoute(QObject *parent = nullptr);

    static APIRoute *instance(void) { return m_instance; }
    void activateRoute(int routeID);

signals:
    void sendNotificationMessage(const QString &url, const QJsonObject &obj);

public slots:
    void handleActivateRouteUrl(const APIRequest &request, APIResponse *response);
    void handleLockRouteUrl(const APIRequest &request, APIResponse *response);
    void handleGetRouteList(const APIRequest &request, APIResponse *response);
    void handleGetRouteEntryList(const APIRequest &request, APIResponse *response);

    void deviceStatusChanged(int deviceID, int status, bool locked);
    bool isRouteActive(int routeID);
    bool isRouteLocked(int routeID) { return m_lockedRoutes.contains(routeID); }
    bool canRouteLock(int routeID) { return !m_excludeRoutes.contains(routeID); }

private:
    void lockRoute(int routeID, bool lock);
    void createAndSendNotificationMessage(int routeID, bool isActive);
    QList<int> m_lockedRoutes;
    QList<int> m_excludeRoutes;

    static APIRoute *m_instance;
};

#endif // APIROUTE_H
