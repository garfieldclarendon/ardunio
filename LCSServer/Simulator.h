#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QJsonArray>

#include "GlobalDefs.h"
#include "UDPMessage.h"

class Simulator : public QObject
{
    Q_OBJECT
public:
    explicit Simulator(QObject *parent = nullptr);

signals:

public slots:
    void newMessage(const UDPMessage &message);
    void timerProc(void);

private:
    void loadData(void);
    void handleActivateRouteMessage(const UDPMessage &message);
    void activateTurnout(int index, TurnoutState newState);
    void activateTurnout(QJsonObject &obj, TurnoutState newState);
    void sendStatus(int deviceID, TurnoutState currentState);

    QJsonArray m_array;
};

#endif // SIMULATOR_H
