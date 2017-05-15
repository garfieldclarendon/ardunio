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

RouteHandler *RouteHandler::m_this = NULL;

RouteHandler::RouteHandler(QObject *parent)
    : QObject(parent)
{
    connect(DeviceManager::instance(), SIGNAL(deviceStatusChanged(int,int)), this, SLOT(deviceStatusChanged(int,int)));
    connect(this, &RouteHandler::sendNotificationMessage, NotificationServer::instance(), &NotificationServer::sendNotificationMessage);
}

RouteHandler *RouteHandler::instance()
{
    if(m_this == NULL)
        m_this = new RouteHandler(qApp);
    return m_this;
}

void RouteHandler::activateRoute(int routeID)
{
    TurnoutHandler *turnoutHandler = qobject_cast<TurnoutHandler *>(DeviceManager::instance()->getHandler(ClassTurnout));
    if(turnoutHandler)
    {
        QString sql = QString("SELECT id, turnoutID, turnoutState FROM routeEntry WHERE routeID = %1").arg(routeID);
        Database db;
        QList<int> turnoutIDs;
        QList<TurnoutState> newStates;

        QSqlQuery query = db.executeQuery(sql);
        while(query.next())
        {
            turnoutIDs << query.value("turnoutID").toInt();
            newStates << (TurnoutState)query.value("turnoutState").toInt();
        }

        for(int x =0; x < turnoutIDs.count(); x++)
        {
            turnoutHandler->activateTurnout(turnoutIDs.value(x), newStates.value(x));
        }
    }
}

void RouteHandler::deviceStatusChanged(int deviceID, int status)
{
    qDebug(QString("RouteHandler::DEVICE_STATUS_CHANGED: %1  STATUS: %2").arg(deviceID).arg(status).toLatin1());
    QString sql = QString("SELECT DISTINCT routeID FROM routeEntry WHERE turnoutID = %1").arg(deviceID);
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
        QString sql = QString("SELECT routeID, turnoutID, turnoutState FROM routeEntry WHERE routeID = %1 ORDER BY routeID").arg(routes.value(x));
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
            int turnoutID = query2.value("turnoutID").toInt();
            int newState = query2.value("turnoutState").toInt();
            if(DeviceManager::instance()->getDeviceStatus(turnoutID) != newState)
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
    QString sql = QString("SELECT turnoutID, turnoutState FROM routeEntry WHERE routeID = %1 ORDER BY routeID").arg(routeID);
    QSqlQuery query2 = db.executeQuery(sql);
    bool isActive = true;
    bool foundRoute = false;
    while(query2.next())
    {
        foundRoute = true;
        int turnoutID = query2.value("turnoutID").toInt();
        int newState = query2.value("turnoutState").toInt();
        if(DeviceManager::instance()->getDeviceStatus(turnoutID) != newState)
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
    obj["routeID"] = routeID;
    obj["isActive"] = isActive;

    emit sendNotificationMessage(uri, obj);
}
