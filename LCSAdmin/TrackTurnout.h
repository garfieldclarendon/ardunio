#ifndef TRACKTURNOUT_H
#define TRACKTURNOUT_H

#include "TrackSegment.h"
#include "GlobalDefs.h"

class DeviceModel;

class TrackTurnout : public TrackSegment
{
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(QObject * model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int deviceID READ deviceID WRITE setDeviceID NOTIFY deviceIDChanged)
    Q_PROPERTY(int blockStatus READ blockStatus WRITE setBlockStatus NOTIFY blockStatusChanged)
    Q_PROPERTY(int turnoutState READ turnoutState WRITE setTurnoutState NOTIFY turnoutStateChanged)

public:
    TrackTurnout();

    bool isActive(void) const { return m_isActive; }
    void setIsActive(bool value) { m_isActive = value; emit isActiveChanged(); }
    QObject *model(void);
    void setModel(QObject *value);
    int deviceID(void) const { return m_deviceID; }
    void setDeviceID(int value);
    BlockState blockStatus(void) const { return m_blockState; }
    void setBlockStatus(int value);
    TurnoutState turnoutState(void) const { return m_state; }
    void setTurnoutState(int value);

    void paintSegment(QPainter *painter) override;

signals:
    void isActiveChanged(void);
    void modelChanged(void);
    void deviceIDChanged(void);
    void blockStatusChanged(void);
    void turnoutStateChanged(void);

protected slots:
    void deviceModelChanged(void);

private:
    void paintTurnoutTopLeft(QPainter *painter);

    bool m_isActive;
    BlockState m_blockState;
    TurnoutState m_state;
    int m_deviceID;
    DeviceModel *m_model;

    // TrackSegment interface
protected:
    void paintText(QPainter *painter) override;
};

#endif // TRACKSINGLE_H
