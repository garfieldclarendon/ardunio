#include <QPainter>

#include "TrackSiding.h"
#include "DeviceModel.h"

TrackSiding::TrackSiding()
    : m_sidingDeviceID(0), m_blockDeviceID(0), m_blockSidingBlockID(0), m_isActive(false), m_blockState(BlockUnknown), m_model(NULL)
{
    setTotalTracks(2);
}

QObject *TrackSiding::model()
{
    return m_model;
}

void TrackSiding::paintSegment(QPainter *painter)
{
    paintSingleTrack(painter);
    paintSiding(painter);
    paintText(painter);
}

void TrackSiding::paintSingleTrack(QPainter *painter)
{
    QPen pen;
    if(isActive())
    {
        if(m_blockState == BlockClear)
            pen.setColor(m_activeColor);
        else if(m_blockState == BlockOccupied)
            pen.setColor(m_occupiedColor);
        else
            pen.setColor(m_activeColor);
    }
    else
    {
        if(m_blockState == BlockOccupied)
            pen.setColor(m_occupiedColor);
        else
            pen.setColor(m_lineColor);
    }
    pen.setWidth(m_penWidth);
    painter->setPen(pen);
    int y = getTrackY(2);
    int mid = boundingRect().width() / 2;

    painter->drawLine(m_margin, y, mid - m_margin, y);
    painter->drawLine(mid + m_margin, y, boundingRect().width() - m_margin, y);
}

void TrackSiding::paintSiding(QPainter *painter)
{
    QPen pen;
    if(isActive())
    {
        if(m_blockState == BlockClear)
            pen.setColor(m_activeColor);
        else if(m_blockState == BlockOccupied)
            pen.setColor(m_occupiedColor);
        else
            pen.setColor(m_activeColor);
    }
    else
    {
        if(m_blockState == BlockOccupied)
            pen.setColor(m_occupiedColor);
        else
            pen.setColor(m_lineColor);
    }
    pen.setWidth(m_penWidth);
    painter->setPen(pen);
    int y = getTrackY(1);
    int y2 = getTrackY(2);
    int mid = boundingRect().width() / 2;

    painter->drawLine(m_margin + 63, y, mid - m_margin, y);
    painter->drawLine(mid + m_margin, y, boundingRect().width() - m_margin - 63, y);

    painter->drawLine(m_margin + 1, y2 - 1, m_margin + 63, y);
    painter->drawLine(boundingRect().width() - m_margin - 62, y, boundingRect().width() - m_margin - 1, y2 - 1);
}

void TrackSiding::setModel(QObject *value)
{
    m_model = qobject_cast<DeviceModel *>(value);

    if(m_model)
    {
        connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(deviceModelChanged()));
        connect(m_model, SIGNAL(modelReset()), this, SLOT(deviceModelChanged()));
        if(m_blockDeviceID > 0 || m_sidingDeviceID > 0)
            deviceModelChanged();
    }
    emit modelChanged();
}

void TrackSiding::setBlockDeviceID(int value)
{
    if(m_blockDeviceID != value)
    {
        m_blockDeviceID = value;
        deviceModelChanged();
        emit blockDeviceIDChanged();
    }
}

void TrackSiding::setSidingBlockID(int value)
{
    if(m_blockSidingBlockID != value)
    {
        m_blockSidingBlockID = value;
        deviceModelChanged();
        emit sidingBlockIDChanged();
    }
}

void TrackSiding::setBlockStatus(int value)
{
    if(m_blockState != value)
    {
        m_blockState = (BlockState)value;
        emit blockStatusChanged();
        update();
    }
}

void TrackSiding::deviceModelChanged()
{
    if(m_model)
    {
        for(int x = 0; x < m_model->rowCount(); x++)
        {
            if(m_model->data(x, "deviceID").toInt() == m_blockDeviceID)
            {
                QString text = m_model->data(x, "deviceName").toString();
                setText(text);
                break;
            }
            else if(m_model->data(x, "deviceID").toInt() == m_sidingDeviceID)
            {
                QString text = m_model->data(x, "deviceName").toString();
                m_sidingText = text;
//                setText(text);
                break;
            }
        }
    }
}
