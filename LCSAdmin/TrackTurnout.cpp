#include <QPainter>

#include "TrackTurnout.h"
#include "DeviceModel.h"

TrackTurnout::TrackTurnout()
    : m_isActive(false), m_blockState(BlockUnknown), m_deviceID(0), m_model(NULL)
{
    setTextAlignment(AlignBottom);
}

QObject *TrackTurnout::model()
{
    return m_model;
}

void TrackTurnout::paintSegment(QPainter *painter)
{
    paintTurnoutTopLeft(painter);
    paintText(painter);
}

void TrackTurnout::setModel(QObject *value)
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

void TrackTurnout::paintTurnoutTopLeft(QPainter *painter)
{
    QPen pen;
    QColor color;
    if(isActive())
    {
        if(m_blockState == BlockClear)
            color = m_activeColor;
        else if(m_blockState == BlockOccupied)
            color = m_occupiedColor;
        else
            color = m_activeColor;
    }
    else
    {
        if(m_blockState == BlockOccupied)
            color = m_occupiedColor;
        else
            color = m_lineColor;
    }
    pen.setWidth(m_penWidth);
    pen.setColor(color);
    painter->setPen(pen);
    int y = getTrackY(1);
    int len = m_margin * 2;
    QPen trimPen;
    trimPen.setWidth(m_penWidth/2);
    trimPen.setColor(m_backgroundColor);

    painter->drawLine(m_margin, y, m_margin + (m_penWidth + m_penWidth/2) + len + 5, y);
    if(m_state == TrnNormal)
       pen.setColor(m_lineColor);
    painter->setPen(pen);
    painter->drawLine(m_margin + (m_penWidth + m_penWidth/2) + len + 5, y - (m_penWidth + m_penWidth/2), len + 60, 0);
    painter->setPen(trimPen);
    painter->drawLine(0, y - m_penWidth, boundingRect().width(), y - m_penWidth);
    if(m_state == TrnDiverging)
        pen.setColor(m_lineColor);
    else
        pen.setColor(color);
    painter->setPen(pen);
    painter->drawLine(m_margin + (m_penWidth + m_penWidth/2) + len + 10, y,boundingRect().width() - m_margin, y);
}

void TrackTurnout::paintText(QPainter *painter)
{
    if(m_text.length() > 0)
    {
        QFont font = painter->font();
        font.setPixelSize(12);
        painter->setPen(m_lineColor);
        painter->setFont(font);

        QRectF b;
        // Get the size of the rectangle first
        painter->drawText(QRect(), Qt::AlignHCenter, m_text, &b);
        QRectF rect(0, m_margin, b.width() + m_margin, b.height());
        if(m_textAlignment == AlignBottom)
        {
            rect.setTop(m_margin + m_top + m_lineHeight/2);
            rect.setHeight(b.height());
            rect.setLeft(m_margin);
        }
        painter->drawText(rect, Qt::AlignHCenter, m_text, NULL);
    }
}

void TrackTurnout::setDeviceID(int value)
{
    if(m_deviceID != value)
    {
        m_deviceID = value;
        deviceModelChanged();
        emit deviceIDChanged();
    }
}

void TrackTurnout::setBlockStatus(int value)
{
    if(m_blockState != value)
    {
        m_blockState = (BlockState)value;
        emit blockStatusChanged();
        update();
    }
}

void TrackTurnout::setTurnoutState(int value)
{
    if(m_state != value)
    {
        m_state = (TurnoutState)value;
        emit turnoutStateChanged();
        update();
    }
}

void TrackTurnout::deviceModelChanged()
{
    if(m_model)
    {
        for(int x = 0; x < m_model->rowCount(); x++)
        {
            if(m_model->data(x, "deviceID").toInt() == m_deviceID)
            {
                QString text = m_model->data(x, "deviceName").toString();
                setText(text);
                TurnoutState s = (TurnoutState)m_model->data(x, "deviceState").toInt();
                setTurnoutState(s);
                break;
            }
        }
    }
}
