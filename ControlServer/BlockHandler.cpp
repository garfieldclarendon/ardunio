#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QVariant>
#include <QTimer>

#include "BlockHandler.h"
#include "Database.h"
#include "ControllerManager.h"
#include "DeviceManager.h"
#include "WebServer.h"

BlockHandler::BlockHandler(QObject *parent)
    : DeviceHandler(ClassBlock, parent)
{
    connect(ControllerManager::instance(), SIGNAL(controllerRemoved(int)), this, SLOT(controllerRemoved(int)));
}

void BlockHandler::newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json)
{
    Q_UNUSED(actionType);
    if(uri == "/controller/module" && classCode == ClassBlock)
        updateBlockState(json, serialNumber, moduleIndex);
}

void BlockHandler::getBlockID(int serialNumber, int moduleIndex, int port, int &blockID)
{
    QString sql = QString("SELECT device.id FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerModuleID WHERE controller.serialNumber = %1 AND controllerModule.moduleIndex = %2 AND device.port = %3").arg(serialNumber).arg(moduleIndex).arg(port);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        blockID = query.value("id").toInt();
    }
}

void BlockHandler::controllerRemoved(int serialNumber)
{
    QString sql = QString("SELECT device.id as blockID FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerMOduleID WHERE serialNumber = %1").arg(serialNumber);

    QList<int> blockIDs;
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        blockIDs << query.value("blockID").toInt();
    }

    for(int x = 0; x < blockIDs.count(); x++)
    {
        int blockID(blockIDs.value(x));
        m_mapMutex.lock();
        m_blockStates[blockID] = BlockUnknown;
        m_mapMutex.unlock();
        DeviceManager::instance()->setDeviceStatus(blockID, BlockUnknown);
        createAndSendNotificationMessage(blockID, BlockUnknown);
    }
}

void BlockHandler::getIPAddressAndModuleIndexForDevice(int deviceID, QString &ipAddress, int &moduleIndex, int &port, int &serialNumber)
{
    QString sql = QString("SELECT serialNumber, controllerModule.moduleIndex, device.port FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerMOduleID WHERE device.id = %1").arg(deviceID);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        serialNumber = query.value("serialNumber").toInt();
        moduleIndex = query.value("moduleIndex").toInt();
        port = query.value("port").toInt();
    }

    ipAddress = ControllerManager::instance()->getControllerIPAddress(serialNumber);
}

void BlockHandler::updateBlockState(const QJsonObject &json, int serialNumber, int moduleIndex)
{
    qDebug(QString("updateBlockState %1 - %2").arg(serialNumber).arg(moduleIndex).toLatin1());
    if(json.contains("blocks"))
    {
        QJsonArray array = json["blocks"].toArray();

        for(int x = 0; x < array.size(); x++)
        {
            QJsonObject obj = array.at(x).toObject();
            int port = obj["port"].toInt();

            int blockID = 0;

            BlockState blockState;

            getBlockID(serialNumber, moduleIndex, port, blockID);

            int i = obj["blockPin"].toInt();
            if(i == 1)
                blockState = BlockOccupied;
            else
                blockState = BlockClear;

            if(blockID > 0 && blockState != BlockUnknown)
            {
                qDebug(QString("UPDATING BLOCK: %1 NewState %2").arg(blockState).arg(blockState).toLatin1());
                setCurrentState(blockID, blockState);
            }
        }
    }
}

void BlockHandler::setCurrentState(int blockID, BlockState newState)
{
    m_mapMutex.lock();
    BlockState current = m_blockStates.value(blockID);
    m_mapMutex.unlock();
    if(current != newState && newState != BlockUnknown)
    {
        m_mapMutex.lock();
        m_blockStates[blockID] = newState;
        m_mapMutex.unlock();
        DeviceManager::instance()->setDeviceStatus(blockID, newState);
        createAndSendNotificationMessage(blockID, newState);
    }
}

void BlockHandler::createAndSendNotificationMessage(int blockID, BlockState newState)
{
    QString uri("/api/notification/block");
    QJsonObject obj;
    obj["blockID"] = QString("%1").arg(blockID);
    obj["state"] = QString("%1").arg(newState);

    emit sendNotificationMessage(uri, obj);
}
