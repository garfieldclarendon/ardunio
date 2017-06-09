#ifndef SEMAPHOREHANDLER_H
#define SEMAPHOREHANDLER_H

#include "DeviceHandler.h"

class SemaphoreHandler : public DeviceHandler
{
    Q_OBJECT
public:
    explicit SemaphoreHandler(QObject *parent = 0);

public slots:
    void deviceStatusChanged(int deviceID, int status);
    void newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);

private:
    void updateSignal(int signalId);
    void sendSignalUpdateMessage(int serialNumber, int moduleIndex, int port, int redMode, int yellowMode, int greenMode);
};

#endif // SEMAPHOREHANDLER_H
