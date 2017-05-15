#ifndef ROUTEHANDLER_H
#define ROUTEHANDLER_H

#include <QObject>

#include "GlobalDefs.h"

class RouteHandler : public QObject
{
    Q_OBJECT
public:
    explicit RouteHandler(QObject *parent = 0);

    static RouteHandler *instance(void);

public slots:
    void activateRoute(int routeID);
    void deviceStatusChanged(int deviceID, int status);
    bool isRouteActive(int routeID);

signals:
    void routeStatusChanged(int routeID, bool isActive);
    void sendNotificationMessage(const QString &uri, QJsonObject &obj);

public slots:

private:
    void createAndSendNotificationMessage(int routeID, bool isActive);

    static RouteHandler *m_this;
};

#endif // ROUTEHANDLER_H
