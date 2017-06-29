#ifndef SIGNALHANDLER_H
#define SIGMALHANDLER_H

#include "DeviceHandler.h"

class SignalHandler : public DeviceHandler
{
    Q_OBJECT
public:
    explicit SignalHandler(QObject *parent = 0);

public slots:
    void deviceStatusChanged(int deviceID, int status);
    void newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);

private:
    void updateSignal(int signalId);
    void sendSignalUpdateMessage(int serialNumber, int moduleIndex, int port, int redMode, int yellowMode, int greenMode);
};

#endif // SIGMALHANDLER_H
