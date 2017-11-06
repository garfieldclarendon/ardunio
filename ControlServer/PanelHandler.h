#ifndef PANELHANDLER_H
#define PANELHANDLER_H

#include "GlobalDefs.h"
#include "DeviceHandler.h"

class PanelHandler : public DeviceHandler
{
    Q_OBJECT

public:
    PanelHandler(QObject *parent = 0);

signals:
    void pinStateChanged(int address, int pinNumber, int pinMode);

public slots:
    void deviceStatusChanged(int deviceID, int status);
    void routeChanged(int routeID, bool isActive);
    void newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);

private:
    int getRouteID(int serialNumber, int address, int pin);
    void fillRouteStatusMap(QMap<int, int> statusMap, int routeID);
};

#endif // PANELHANDLER_H
