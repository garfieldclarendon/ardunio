#include <QPainter>

#include "TrackSingle.h"
#include "DeviceModel.h"

TrackSingle::TrackSingle()
    : m_isActive(false), m_blockState(BlockUnknown), m_deviceID(0), m_model(NULL)
{

}

QObject *TrackSingle::model()
{
    return m_model;
}

void TrackSingle::paintSegment(QPainter *painter)
{
    paintSingleTrack(painter);
    paintText(painter);
}

void TrackSingle::setModel(QObject *value)
{
    m_model = qobject_cast<DeviceModel *>(value);

    if(m_model)
    {
        connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(deviceModelChanged()));
        connect(m_model, SIGNAL(modelReset()), this, SLOT(deviceModelChanged()));
        if(m_deviceID > 0)
            deviceModelChanged();
    }
    emit modelChanged();
}

void TrackSingle::paintSingleTrack(QPainter *painter)
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
    painter->drawLine(m_margin, y, boundingRect().width() - m_margin, y);
}

void TrackSingle::setDeviceID(int value)
{
    if(m_deviceID != value)
    {
        m_deviceID = value;
        deviceModelChanged();
        emit deviceIDChanged();
    }
}

void TrackSingle::setBlockStatus(int value)
{
    if(m_blockState != value)
    {
        m_blockState = (BlockState)value;
        emit blockStatusChanged();
        update();
    }
}

void TrackSingle::deviceModelChanged()
{
    if(m_model)
    {
        for(int x = 0; x < m_model->rowCount(); x++)
        {
            if(m_model->data(x, "deviceID").toInt() == m_deviceID)
            {
                QString text = m_model->data(x, "deviceName").toString();
                setText(text);
                break;
            }
        }
    }
}
