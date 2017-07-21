#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <QObject>
/////////////////////////////////////////////////////////////////////
// The following comment blocks are parsed by the apidoc application to produce the API documentation
/**
 * @api {get} /activate_route/:routeID Activate Route
 * @apiName ActivateRoute
 * @apiGroup Routes
 *
 * @apiParam {Number} routeID The route's ID.
 * @apiDescription Activates the desired route.  When activated, all turnouts are set to the directing as defined in the route configuration.
 * @apiExample Example usage:
 * http://localhost:8080/api/activate_route?routeID=1
 */

/**
 * @api {get} /activate_turnout/:deviceID,turnoutState Activate Turnout
 * @apiName ActivateTurnout
 * @apiGroup Turnouts
 *
 * @apiParam {Number} deviceID The turnout's Device ID.
 * @apiParam {Number=1,3} turnoutState The desired state to set the turnout to.
 * @apiDescription Sets the turnout to the desired state (direction) 1 = Normal 2 = Diverging
 * @apiExample Example usage:
 * http://localhost:8080/api/activate_turnout?deviceID=7&turnoutState=3
 */

/**
 * @api {get} /panel_list/ Get a list of Panels
 * @apiName PanelList
 * @apiGroup Panels
 *
 * @apiDescription Returns a list of defined panels
 * @apiSuccess {Object[]} panels       List of panels.
 * @apiSuccess {Number} panels.moduleIndex Index of the module.
 * @apiSuccess {Number} panels.panelID
 * @apiSuccess {String} panels.panelName Descriptive name of the panel.
 * @apiSuccess {Number} panels.serialNumber Controller's serial number.
 * @apiExample Example usage:
 * http://localhost:8080/api/panel_list
 * @apiSuccessExample {json} Success-Response:
 *     HTTP/1.1 200 OK
 *   [
 *      {
 *          "moduleIndex": "0",
 *          "panelID": "2",
 *          "panelName": "CA Panel1",
 *          "serialNumber": "637866"
 *      },
 *      {
 *          "moduleIndex": "0",
 *          "panelID": "1",
 *          "panelName": "Gaskill Mine",
 *          "serialNumber": "1551385"
 *      },
 *   ]
 */

/**
 * @api {get} /panel_routes/:panelID Get a list of Routes assigned to a Panel
 * @apiName PanelRoutes
 * @apiGroup Panels
 *
 * @apiParam {Number} panelID The panel's ID.
 * @apiDescription Returns a list of routes (buttons) assigned to a panel
 * @apiSuccess {Object[]} routes       List of routes assigned to the panel.
 * @apiSuccess {Bool} routes.isActive indicates if the route is active.
 * @apiSuccess {String} routes.routeDescription Descriptive name of the route
 * @apiSuccess {Number} routes.routeID The route's ID.
 * @apiSuccess {String} routes.routeName Route's name.
 * @apiExample Example usage:
 * http://localhost:8080/api/panel_routes?panelID=1
 * @apiSuccessExample {json} Success-Response:
 *     HTTP/1.1 200 OK
 *   [
 *      {
 *          "isActive": "false",
 *          "routeDescription": "Gaskill Mine Route 1",
 *          "routeID": "1",
 *          "routeName": "GM 1"
 *      },
 *      {
 *          "isActive": "true",
 *          "routeDescription": "Gaskill Mine Route 2",
 *          "routeID": "2",
 *          "routeName": "GM 2"
 *      },
 *   ]
 */

// End API documentation
////////////////////////////////////////////////////////////////////

class QTcpSocket;
class QUrl;

class APIHandler : public QObject
{
    Q_OBJECT
public:
    explicit APIHandler(QObject *parent = 0);

    void handleClient(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload);

signals:

public slots:
private:
    void handleActivateTurnout(QTcpSocket *socket, const QUrl &url, const QString &actionText, const QString &payload);
    void handleActivateRoute(QTcpSocket *socket, const QUrl &url, const QString &actionText, const QString &payload);
    void handleGetPanelRouteList(QTcpSocket *socket, const QUrl &url);
    void handleGetPanelList(QTcpSocket *socket, const QUrl &url);
    void handleGetDeviceList(QTcpSocket *socket, const QUrl &url);
    void handleGetControllerList(QTcpSocket *socket, const QUrl &url);
    void handleSendModuleConfig(QTcpSocket *socket, const QUrl &url);
    void handleControllerFirmwareUpdate(QTcpSocket *socket, const QUrl &url);
    void handleControllerReset(QTcpSocket *socket, const QUrl &url);
    void handleControllerResetConfig(QTcpSocket *socket, const QUrl &url);
};

#endif // APIHANDLER_H
