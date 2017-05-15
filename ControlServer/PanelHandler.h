#ifndef PANELHANDLER_H
#define PANELHANDLER_H

#include "GlobalDefs.h"
#include "DeviceHandler.h"

class PanelHandler : public DeviceHandler
{
    Q_OBJECT

public:
    PanelHandler(QObject *parent = 0);

public slots:
    void deviceStatusChanged(int deviceID, int status);
    void newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);

private:
    int getRouteID(int serialNumber, int moduleIndex, int buttonIndex);

};

#endif // PANELHANDLER_H
