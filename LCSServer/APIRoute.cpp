#include <QUrl>
#include <QUrlQuery>
#include <QSqlQuery>
#include <QJsonArray>
#include <QJsonDocument>

#include "APIRoute.h"

#include "APITurnout.h"
#include "MessageBroadcaster.h"
#include "WebServer.h"
#include "Database.h"
#include "DeviceManager.h"
#include "NotificationServer.h"

APIRoute *APIRoute::m_instance = NULL;

APIRoute::APIRoute(QObject *parent) : QObject(parent)
{
    m_instance = this;
    connect(this, &APIRoute::sendNotificationMessage, NotificationServer::instance(), &NotificationServer::sendNotificationMessage);
    connect(DeviceManager::instance(), &DeviceManager::deviceStatusChanged, this, &APIRoute::deviceStatusChanged);

    WebServer *webServer = WebServer::instance();

    UrlHandler *handler;

    handler = webServer->createUrlHandler("/api/activate_route");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleActivateRouteUrl(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/lock_route");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleLockRouteUrl(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/route_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetRouteList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/route_entry_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetRouteEntryList(APIRequest,APIResponse*)), Qt::DirectConnection);
}

void APIRoute::activateRoute(int routeID)
{
    UDPMessage message;
    message.setMessageID(TRN_ACTIVATE_ROUTE);
    message.setID(routeID);
    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void APIRoute::handleActivateRouteUrl(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int routeID = urlQuery.queryItemValue("routeID").toInt();

    activateRoute(routeID);
}

void APIRoute::handleLockRouteUrl(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int routeID = urlQuery.queryItemValue("routeID").toInt();
    bool lock= urlQuery.queryItemValue("lock").toInt() == 1;

    lockRoute(routeID, lock);
}

void APIRoute::handleGetRouteList(const APIRequest &, APIResponse *response)
{
    qDebug(QString("handleGetRouteList.").toLatin1());
    Database db;

    QString sql = QString("SELECT ID as routeID, routeName, routeDescription FROM Route ORDER BY routeName");
    QJsonArray jsonArray = db.fetchItems(sql);

    for(int x = 0; x < jsonArray.size(); x++)
    {
        QJsonObject obj = jsonArray[x].toObject();
        QString routeID = obj["routeID"].toString();
        bool isActive = isRouteActive(routeID.toInt());
        bool isLocked = isRouteLocked(routeID.toInt());
        bool canLock = canRouteLock(routeID.toInt());
        obj["routeID"] = routeID;
        obj["isActive"] = isActive;
        obj["isLocked"] = isLocked;
        obj["canLock"] = canLock;
        jsonArray[x] = obj;
    }

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APIRoute::handleGetRouteEntryList(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
    int routeID = urlQuery.queryItemValue("routeID").toInt();

    qDebug(QString("handleGetRouteEntryList.  routeID = %1").arg(routeID).toLatin1());
    Database db;

    QString sql = QString("SELECT ID as routeEntryID, routeID, deviceID, turnoutState FROM RouteEntry WHERE routeID = %1").arg(routeID);
    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APIRoute::deviceStatusChanged(int deviceID, int status, bool locked)
{
    qDebug(QString("RouteHandler::DEVICE_STATUS_CHANGED: %1  STATUS: %2 LOCKED: %3").arg(deviceID).arg(status).arg(locked).toLatin1());
    QString sql = QString("SELECT DISTINCT routeID FROM routeEntry WHERE deviceID = %1").arg(deviceID);
    Database db;
    QList<int> routes;

    {
        QSqlQuery query = db.executeQuery(sql);
        while(query.next())
        {
            routes << query.value("routeID").toInt();
        }
    }

    for(int x = 0; x < routes.count(); x++)
    {
        int currentRoute = 0;
        bool isActive = true;
        bool isLocked = true;

        QString sql = QString("SELECT routeID, deviceID, turnoutState FROM routeEntry WHERE routeID = %1 ORDER BY routeID").arg(routes.value(x));
        QSqlQuery query2 = db.executeQuery(sql);
        while(query2.next())
        {
            if(query2.value("routeID").toInt() != currentRoute)
            {
                if(currentRoute > 0)
                {
                    if(isLocked && m_lockedRoutes.contains(currentRoute) == false)
                        m_lockedRoutes << currentRoute;

                    createAndSendNotificationMessage(currentRoute, isActive);
                    isActive = true;
                    isLocked = true;
                }
                currentRoute = query2.value("routeID").toInt();
            }
            int deviceID = query2.value("deviceID").toInt();
            int newState = query2.value("turnoutState").toInt();
            if(DeviceManager::instance()->getDeviceStatus(deviceID) != newState)
                isActive = false;
            if(DeviceManager::instance()->getIsDeviceLocked(deviceID) == false)
                isLocked = false;
        }
        if(currentRoute > 0)
        {
            if(isLocked && m_lockedRoutes.contains(currentRoute) == false)
                m_lockedRoutes << currentRoute;
            createAndSendNotificationMessage(currentRoute, isActive);
        }
    }
}

bool APIRoute::isRouteActive(int routeID)
{
    Database db;
    QString sql = QString("SELECT deviceID, turnoutState FROM routeEntry WHERE routeID = %1").arg(routeID);
    QSqlQuery query2 = db.executeQuery(sql);
    bool isActive = true;
    bool foundRoute = false;
    while(query2.next())
    {
        foundRoute = true;
        int deviceID = query2.value("deviceID").toInt();
        int newState = query2.value("turnoutState").toInt();
        if(DeviceManager::instance()->getDeviceStatus(deviceID) != newState)
            isActive = false;
    }
    if(!foundRoute)
        isActive = false;
    return isActive;
}

void APIRoute::createAndSendNotificationMessage(int routeID, bool isActive)
{
    QString uri("/api/notification/route");
    QJsonObject obj;
    bool isLocked = isRouteLocked(routeID);
    bool canLock = canRouteLock(routeID);

    obj["routeID"] = routeID;
    obj["isActive"] = isActive;
    obj["isLocked"] = isLocked;
    obj["canLock"] = canLock;

    emit sendNotificationMessage(uri, obj);
}

void APIRoute::lockRoute(int routeID, bool lock)
{
    if(lock)
    {
        if(m_lockedRoutes.contains(routeID) == false)
            m_lockedRoutes << routeID;
        Database db;
        QList<int> excludeList = db.getExcludeRouteList(routeID);
        m_excludeRoutes.append(excludeList);
        for(int x = 0; x < m_excludeRoutes.count(); x++)
        {
            if(excludeList.value(x) != routeID)
                createAndSendNotificationMessage(m_excludeRoutes.value(x), false);
        }
    }
    else
    {
        m_lockedRoutes.removeAll(routeID);
        Database db;
        QList<int> excludeList = db.getExcludeRouteList(routeID);
        for(int x = 0; x < excludeList.count(); x++)
        {
            m_excludeRoutes.removeAll(excludeList.value(x));
            if(excludeList.value(x) != routeID)
                createAndSendNotificationMessage(excludeList.value(x), false);
        }
    }

    MessageBroadcaster::instance()->sendLockRouteCommand(routeID, lock);
    bool isActive = isRouteActive(routeID);
    createAndSendNotificationMessage(routeID, isActive);
}
