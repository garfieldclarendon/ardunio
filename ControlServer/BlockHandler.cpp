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

void BlockHandler::newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json)
{
    Q_UNUSED(actionType);
    if(uri == "/controller/module" && classCode == ClassBlock)
        updateBlockState(json, serialNumber, address);
}

void BlockHandler::getdeviceID(int serialNumber, int address, int port, int &deviceID)
{
    QString sql = QString("SELECT device.id FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerModuleID WHERE controller.serialNumber = %1 AND controllerModule.address = %2 AND device.port = %3").arg(serialNumber).arg(address).arg(port);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        deviceID = query.value("id").toInt();
    }
}

void BlockHandler::controllerRemoved(int serialNumber)
{
    QString sql = QString("SELECT device.id as deviceID FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerMOduleID WHERE serialNumber = %1").arg(serialNumber);

    QList<int> deviceIDs;
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        deviceIDs << query.value("deviceID").toInt();
    }

    for(int x = 0; x < deviceIDs.count(); x++)
    {
        int deviceID(deviceIDs.value(x));
        m_mapMutex.lock();
        m_blockStates[deviceID] = BlockUnknown;
        m_mapMutex.unlock();
        DeviceManager::instance()->setDeviceStatus(deviceID, BlockUnknown);
        createAndSendNotificationMessage(deviceID, BlockUnknown);
    }
}

void BlockHandler::getIPAddressAndaddressForDevice(int deviceID, QString &ipAddress, int &address, int &port, int &serialNumber)
{
    QString sql = QString("SELECT serialNumber, controllerModule.address, device.port FROM controllerModule JOIN controller ON controllerModule.controllerID = controller.ID JOIN device ON controllerModule.id = device.controllerMOduleID WHERE device.id = %1").arg(deviceID);
    Database db;
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        serialNumber = query.value("serialNumber").toInt();
        address = query.value("address").toInt();
        port = query.value("port").toInt();
    }

    ipAddress = ControllerManager::instance()->getControllerIPAddress(serialNumber);
}

void BlockHandler::updateBlockState(const QJsonObject &json, int serialNumber, int address)
{
    qDebug(QString("updateBlockState %1 - %2").arg(serialNumber).arg(address).toLatin1());
    if(json.contains("blocks"))
    {
        QJsonArray array = json["blocks"].toArray();

        for(int x = 0; x < array.size(); x++)
        {
            QJsonObject obj = array.at(x).toObject();
            int port = obj["port"].toInt();

            int deviceID = 0;

            BlockState blockState;

            getdeviceID(serialNumber, address, port, deviceID);

            int i = obj["blockPin"].toInt();
            if(i == 1)
                blockState = BlockOccupied;
            else
                blockState = BlockClear;

            if(deviceID > 0 && blockState != BlockUnknown)
            {
                qDebug(QString("UPDATING BLOCK: %1 NewState %2").arg(blockState).arg(blockState).toLatin1());
                setCurrentState(deviceID, blockState);
            }
        }
    }
}

void BlockHandler::setCurrentState(int deviceID, BlockState newState)
{
    m_mapMutex.lock();
    BlockState current = m_blockStates.value(deviceID);
    m_mapMutex.unlock();
    if(current != newState && newState != BlockUnknown)
    {
        m_mapMutex.lock();
        m_blockStates[deviceID] = newState;
        m_mapMutex.unlock();
        DeviceManager::instance()->setDeviceStatus(deviceID, newState);
        createAndSendNotificationMessage(deviceID, newState);
    }
}

void BlockHandler::createAndSendNotificationMessage(int deviceID, BlockState newState)
{
    QString uri("/api/notification/block");
    QJsonObject obj;
    obj["deviceID"] = QString("%1").arg(deviceID);
    obj["state"] = QString("%1").arg(newState);

    emit sendNotificationMessage(uri, obj);
}
