#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QVariant>
#include <QCoreApplication>

#include "RouteHandler.h"
#include "Database.h"
#include "DeviceManager.h"
#include "TurnoutHandler.h"
#include "NotificationServer.h"
#include "MessageBroadcaster.h"

RouteHandler *RouteHandler::m_this = NULL;

RouteHandler::RouteHandler(QObject *parent)
    : QObject(parent)
{
    connect(this, SIGNAL(sendNotificationMessage(QString,QJsonObject)), NotificationServer::instance(), SLOT(sendNotificationMessage(QString,QJsonObject)), Qt::QueuedConnection);
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)), Qt::QueuedConnection);
//    connect(this, &RouteHandler::sendNotificationMessage, NotificationServer::instance(), &NotificationServer::sendNotificationMessage);
}

RouteHandler *RouteHandler::instance()
{
    if(m_this == NULL)
        m_this = new RouteHandler(qApp);
    return m_this;
}

void RouteHandler::activateRoute(int routeID)
{
//    TurnoutHandler *turnoutHandler = qobject_cast<TurnoutHandler *>(DeviceManager::instance()->getHandler(DeviceTurnout));
//    if(turnoutHandler)
//    {
//        QString sql = QString("SELECT id, deviceID, turnoutState FROM routeEntry WHERE routeID = %1").arg(routeID);
//        Database db;
//        QList<int> deviceIDs;
//        QList<TurnoutState> newStates;

//        QSqlQuery query = db.executeQuery(sql);
//        while(query.next())
//        {
//            deviceIDs << query.value("deviceID").toInt();
//            newStates << (TurnoutState)query.value("turnoutState").toInt();
//        }

//        for(int x =0; x < deviceIDs.count(); x++)
//        {
//            turnoutHandler->activateTurnout(deviceIDs.value(x), newStates.value(x));
//        }
//    }
    UDPMessage message;
    message.setMessageID(TRN_ACTIVATE_ROUTE);
    message.setID(routeID);
    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void RouteHandler::lockRoute(int routeID, bool lock)
{
//    TurnoutHandler *turnoutHandler = qobject_cast<TurnoutHandler *>(DeviceManager::instance()->getHandler(DeviceTurnout));
//    if(turnoutHandler)
//    {
//        QString sql = QString("SELECT id, deviceID, turnoutState FROM routeEntry WHERE routeID = %1").arg(routeID);
//        Database db;
//        QList<int> deviceIDs;
//        QList<TurnoutState> newStates;

//        QSqlQuery query = db.executeQuery(sql);
//        while(query.next())
//        {
//            deviceIDs << query.value("deviceID").toInt();
//            newStates << (TurnoutState)query.value("turnoutState").toInt();
//        }

//        for(int x =0; x < deviceIDs.count(); x++)
//        {
//            turnoutHandler->activateTurnout(deviceIDs.value(x), newStates.value(x));
//        }
//    }
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

void RouteHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("RouteHandler::DEVICE_STATUS_CHANGED: %1  STATUS: %2").arg(deviceID).arg(status).toLatin1());
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

    int currentRoute = 0;
    bool isActive = true;
    for(int x = 0; x < routes.count(); x++)
    {
        QString sql = QString("SELECT routeID, deviceID, turnoutState FROM routeEntry WHERE routeID = %1 ORDER BY routeID").arg(routes.value(x));
        QSqlQuery query2 = db.executeQuery(sql);
        while(query2.next())
        {
            if(query2.value("routeID").toInt() != currentRoute)
            {
                if(currentRoute > 0)
                {
                    emit routeStatusChanged(currentRoute, isActive);
                    createAndSendNotificationMessage(currentRoute, isActive);
                    isActive = true;
                }
                currentRoute = query2.value("routeID").toInt();
            }
            int deviceID = query2.value("deviceID").toInt();
            int newState = query2.value("turnoutState").toInt();
            if(DeviceManager::instance()->getDeviceStatus(deviceID) != newState)
                isActive = false;
        }
        if(currentRoute > 0)
        {
            emit routeStatusChanged(currentRoute, isActive);
            createAndSendNotificationMessage(currentRoute, isActive);
        }
    }
}

bool RouteHandler::isRouteActive(int routeID)
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

void RouteHandler::createAndSendNotificationMessage(int routeID, bool isActive)
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
