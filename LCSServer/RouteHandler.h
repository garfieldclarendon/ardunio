#ifndef ROUTEHANDLER_H
#define ROUTEHANDLER_H

#include <QObject>
#include <QJsonObject>

#include "GlobalDefs.h"

class RouteHandler : public QObject
{
    Q_OBJECT
public:
    explicit RouteHandler(QObject *parent = 0);

    static RouteHandler *instance(void);

public slots:
    void activateRoute(int routeID);
    void lockRoute(int routeID, bool lock);
    void deviceStatusChanged(int deviceID, int status);
    bool isRouteActive(int routeID);
    bool isRouteLocked(int routeID) { return m_lockedRoutes.contains(routeID); }
    bool canRouteLock(int routeID) { return !m_excludeRoutes.contains(routeID); }

signals:
    void routeStatusChanged(int routeID, bool isActive);
    void sendNotificationMessage(const QString &uri, const QJsonObject &obj);

public slots:

private:
    void createAndSendNotificationMessage(int routeID, bool isActive);

    static RouteHandler *m_this;
    QList<int> m_lockedRoutes;
    QList<int> m_excludeRoutes;
};

#endif // ROUTEHANDLER_H
