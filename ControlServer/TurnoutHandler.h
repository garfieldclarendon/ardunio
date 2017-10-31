#ifndef TURNOUTHANDLER_H
#define TURNOUTHANDLER_H

#include <QMap>
#include <QMutex>

#include "GlobalDefs.h"
#include "DeviceHandler.h"
#include "controllermessage.h"

class TurnoutHandler : public DeviceHandler
{
    Q_OBJECT
public:
    explicit TurnoutHandler(QObject *parent = 0);

    // DeviceHandler interface
public:

signals:
    void sendNotificationMessage(const QString &uri, QJsonObject &obj);

public slots:
    void newMessage(int serialNumber, int address, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);
    void activateTurnout(int deviceID, TurnoutState newState);
    void timerProc(void);
    void sendConfig(int serialNumber, int address);
    void controllerRemoved(int serialNumber);
    void controllerConnected(int index);

    void messageACKed(const ControllerMessage &message);

private:
    int getMotorPinSetting(int deviceID);
    void getdeviceIDAndMotorSetting(int serialNumber, int address, int port, int &deviceID, int &motorPinSetting);
    void getIPAddressAndaddressForDevice(int deviceID, QString &ipAddress, int &address, int &port, int &serialNumber);
    void updateTurnoutState(const QJsonObject &json, int serialNumber, int address);
    void setCurrentState(int deviceID, TurnoutState newState);
    void createAndSendNotificationMessage(int deviceID, TurnoutState newState);

    TurnoutState getTurnoutState(const QJsonObject &obj, int motorPinSetting) const;

    QMutex m_mapMutex;
    QMap<int, TurnoutState> m_turnoutStates;
};

#endif // TURNOUTHANDLER_H
