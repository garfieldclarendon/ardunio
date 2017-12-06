#ifndef TRACKSIDING_H
#define TRACKSIDING_H

#include "TrackSegment.h"
#include "GlobalDefs.h"

class DeviceModel;

class TrackSiding : public TrackSegment
{
    Q_OBJECT
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)
    Q_PROPERTY(QObject * model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int blockDeviceID READ blockDeviceID WRITE setBlockDeviceID NOTIFY blockDeviceIDChanged)
    Q_PROPERTY(int sidingBlockID READ sidingBlockID WRITE setSidingBlockID NOTIFY sidingBlockIDChanged)
    Q_PROPERTY(int blockStatus READ blockStatus WRITE setBlockStatus NOTIFY blockStatusChanged)

public:
    TrackSiding();

    bool isActive(void) const { return m_isActive; }
    void setIsActive(bool value) { m_isActive = value; emit isActiveChanged(); }
    QObject *model(void);
    void setModel(QObject *value);
    int blockDeviceID(void) const { return m_blockDeviceID; }
    void setBlockDeviceID(int value);
    int sidingBlockID(void) const { return m_blockSidingBlockID; }
    void setSidingBlockID(int value);
    BlockState blockStatus(void) const { return m_blockState; }
    void setBlockStatus(int value);

signals:
    void isActiveChanged(void);
    void modelChanged(void);
    void blockDeviceIDChanged(void);
    void sidingBlockIDChanged(void);
    void blockStatusChanged(void);

protected slots:
    void deviceModelChanged(void);

    void paintSegment(QPainter *painter) override;

private:
    void paintSingleTrack(QPainter *painter);
    void paintSiding(QPainter *painter);

    QString m_sidingText;
    int m_sidingDeviceID;
    int m_blockDeviceID;
    int m_blockSidingBlockID;
    bool m_isActive;
    BlockState m_blockState;
    DeviceModel *m_model;
};

#endif // TRACKSIDING_H
