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
    void newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);
    void controllerRemoved(int serialNumber);

private:
    void getBlockID(int serialNumber, int moduleIndex, int port, int &blockID);
    void getIPAddressAndModuleIndexForDevice(int deviceID, QString &ipAddress, int &moduleIndex, int &port, int &serialNumber);
    void setCurrentState(int blockID, BlockState newState);
    void createAndSendNotificationMessage(int blockID, BlockState newState);
    void updateBlockState(const QJsonObject &json, int serialNumber, int moduleIndex);

    QMutex m_mapMutex;
    QMap<int, BlockState> m_blockStates;
};

#endif // BLOCKHANDLER_H
