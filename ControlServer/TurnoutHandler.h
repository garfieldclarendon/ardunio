#ifndef TURNOUTHANDLER_H
#define TURNOUTHANDLER_H

#include <QMap>
#include <QMutex>

#include "GlobalDefs.h"
#include "DeviceHandler.h"

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
    void newMessage(int serialNumber, int moduleIndex, ClassEnum classCode, NetActionType actionType, const QString &uri, const QJsonObject &json);
    void activateTurnout(int deviceID, TurnoutState newState);
    void timerProc(void);
    void sendConfig(int serialNumber, int moduleIndex);
    void controllerRemoved(int serialNumber);

private:
    int getMotorPinSetting(int turnoutID);
    void getTurnoutIDAndMotorSetting(int serialNumber, int moduleIndex, int port, int &turnoutID, int &motorPinSetting);
    void getIPAddressAndModuleIndexForDevice(int deviceID, QString &ipAddress, int &moduleIndex, int &port, int &serialNumber);
    void updateTurnoutState(const QJsonObject &json, int serialNumber, int moduleIndex);
    void setCurrentState(int turnoutID, TurnoutState newState);
    void createAndSendNotificationMessage(int turnoutID, TurnoutState newState);

    TurnoutState getTurnoutState(const QJsonObject &obj, int motorPinSetting) const;

    QMutex m_mapMutex;
    QMap<int, TurnoutState> m_turnoutStates;
};

#endif // TURNOUTHANDLER_H
