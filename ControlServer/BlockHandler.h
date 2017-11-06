#ifndef BLOCKHANDLER_H
#define BLOCKHANDLER_H

#include <QMap>
#include <QMutex>

#include "GlobalDefs.h"
#include "DeviceHandler.h"

class BlockHandler : public DeviceHandler
{
    Q_OBJECT
public:
    explicit BlockHandler(QObject *parent = 0);

    // DeviceHandler interface
public:

signals:
    void sendNotificationMessage(const QString &uri, QJsonObject &obj);

public slots:
    void newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);
    void controllerRemoved(int serialNumber);

private:
    void getdeviceID(int serialNumber, int address, int port, int &deviceID);
    void setCurrentState(int deviceID, BlockState newState);
    void createAndSendNotificationMessage(int deviceID, BlockState newState);
    void updateBlockState(const QJsonObject &json, int serialNumber, int address);

    QMutex m_mapMutex;
    QMap<int, BlockState> m_blockStates;
};

#endif // BLOCKHANDLER_H
