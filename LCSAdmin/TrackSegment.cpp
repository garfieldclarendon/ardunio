#include <QPainter>
#include <QFont>

#include "TrackSegment.h"
#include "GlobalDefs.h"

TrackSegment::TrackSegment(QQuickItem *parent)
 : QQuickPaintedItem(parent), m_top(0), m_penWidth(5), m_textAlignment(AlignTop), m_segmentAlignment(AlignTop), m_backgroundColor(Qt::black), m_activeColor(Qt::green), m_lineColor(Qt::white), m_occupiedColor(Qt::yellow), m_totalTracks(1)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    updateHeight();
}

void TrackSegment::setText(const QString &value)
{
    if(m_text != value)
    {
        m_text = value;
        emit textChanged();
        update();
    }
}

void TrackSegment::paint(QPainter *painter)
{
    painter->setRenderHints(QPainter::Antialiasing, true);
    painter->fillRect(boundingRect(), m_backgroundColor);
    paintSegment(painter);
    trim(painter);
}

void TrackSegment::paintText(QPainter *painter)
{
    QFont font = painter->font();
    font.setPixelSize(12);
    painter->setPen(m_lineColor);
    painter->setFont(font);

    QRectF b;
    // Get the size of the rectangle first
    painter->drawText(QRect(), Qt::AlignHCenter, m_text, &b);
    QRectF rect(0, m_margin, boundingRect().width(), b.height());
    if(m_textAlignment == AlignBottom)
    {
        rect.setTop(m_top + m_lineHeight);
        rect.setBottom(boundingRect().bottom() + m_margin);
    }
    painter->drawText(rect, Qt::AlignHCenter, m_text, NULL);
}

void TrackSegment::updateHeight()
{
    setImplicitHeight( m_margin + (m_lineHeight * m_totalTracks) + m_margin);
}

int TrackSegment::getTrackY(int track)
{
    int y;
    y = m_top + m_lineHeight * track - (m_lineHeight / 2);

    return y;
}

void TrackSegment::trim(QPainter *painter)
{
    QRectF top, bottom, left, right;
    top = boundingRect();
    top.setHeight(m_margin);
    bottom = top;
    bottom.setX(boundingRect().height() - m_margin);

    left = boundingRect();
    left.setWidth(m_margin);
    left.setHeight(boundingRect().height());
    right = left;
    right.setY(boundingRect().width() - m_margin);

    painter->fillRect(top, m_backgroundColor);
    painter->fillRect(bottom, m_backgroundColor);
    painter->fillRect(left, m_backgroundColor);
    painter->fillRect(right, m_backgroundColor);
}


void TrackSegment::paintDoubleTrack(QPainter *painter)
{
    QPen pen(Qt::black);
    pen.setWidth(m_penWidth);
    painter->setPen(pen);
    painter->drawLine(0, m_top, boundingRect().width() - m_margin, m_top);
    painter->drawLine(0, m_top + m_lineHeight, boundingRect().width() - m_margin, m_top + m_lineHeight);
}

void TrackSegment::paintCrossOverRight(QPainter *painter)
{
//    QPen pen;
//    pen.setWidth(m_penWidth);
//    painter->setPen(pen);
//    if(isActive())
//    {
//        pen.setColor(Qt::black);
//        painter->setPen(pen);
//        painter->drawLine(0, m_top, boundingRect().width() / 2 + 60, m_top);
//        pen.setColor(Qt::green);
//        painter->setPen(pen);
//        painter->drawLine(boundingRect().width() / 2 + 60 + m_penWidth - 3, m_top, boundingRect().width() - m_margin, m_top);
//    }
//    else
//    {
//        pen.setColor(Qt::black);
//        painter->setPen(pen);
//        painter->drawLine(0, m_top, boundingRect().width() - m_margin, m_top);
//    }
//    if(isActive())
//    {
//        pen.setColor(Qt::green);
//        painter->setPen(pen);
//        painter->drawLine(0, m_top + m_lineSpacing, boundingRect().width() / 2 - 60, m_top + m_lineSpacing);
//        pen.setColor(Qt::black);
//        painter->setPen(pen);
//        painter->drawLine(boundingRect().width() / 2 - 60 + m_penWidth, m_top + m_lineSpacing, boundingRect().width() - m_margin, m_top + m_lineSpacing);
//    }
//    else
//    {
//        pen.setColor(Qt::black);
//        painter->setPen(pen);
//        painter->drawLine(0, m_top + m_lineSpacing, boundingRect().width() - m_margin, m_top + m_lineSpacing);
//    }
//    pen.setColor(isActive() ? Qt::green : Qt::black);
//    painter->setPen(pen);
//    painter->drawLine(boundingRect().width() / 2 - 60, m_top + m_lineSpacing, boundingRect().width() / 2 + 60, m_top);
}

void TrackSegment::paintCrossOverLeft(QPainter *painter)
{
    QPen pen(Qt::black);
    pen.setWidth(m_penWidth);
    painter->setPen(pen);
    painter->drawLine(0, m_top, boundingRect().width() - m_margin, m_top);
    painter->drawLine(boundingRect().width() / 2 - 60, m_top, boundingRect().width() / 2 + 60, m_top + m_lineHeight);
    painter->drawLine(0, m_top + m_lineHeight, boundingRect().width() - m_margin, m_top + m_lineHeight);
}

void TrackSegment::paintTurnoutRightNorth(QPainter *painter)
{
//    QPen pen;
//    pen.setWidth(m_penWidth);
//    painter->setPen(pen);

//    if(isActive())
//    {
//        if(m_isThrown == false)
//        {
//            pen.setColor(Qt::black);
//            painter->setPen(pen);
//            painter->drawLine(boundingRect().width() / 2 - 30, m_top, boundingRect().width() / 2 + 30, 0);
//        }

//        pen.setColor(Qt::green);
//        painter->setPen(pen);
//        painter->drawLine(0, m_top, boundingRect().width() / 2 + 60, m_top);
//        pen.setColor(m_isThrown ? Qt::black : Qt::green);
//        painter->setPen(pen);
//        painter->drawLine(boundingRect().width() / 2 - 30, m_top, boundingRect().width() - m_margin, m_top);
//        if(m_isThrown)
//        {
//            pen.setColor(Qt::green);
//            painter->setPen(pen);
//            painter->drawLine(boundingRect().width() / 2 - 30, m_top, boundingRect().width() / 2 + 30, 0);
//        }
//    }
//    else
//    {
//        pen.setColor(Qt::black);
//        painter->setPen(pen);
//        painter->drawLine(0, m_top, boundingRect().width() - m_margin, m_top);
//    }
//    if(isActive() == false)
//    {
//        pen.setColor(Qt::black);
//        painter->setPen(pen);
//        painter->drawLine(boundingRect().width() / 2 - 30, m_top, boundingRect().width() / 2 + 30, 0);
//    }
//    QRect r(boundingRect().width() / 2 - 34, m_top - 4, 8, 8);
//    pen.setColor(Qt::black);
//    pen.setWidth(1);
//    painter->setPen(pen);
    //    painter->fillRect(r, m_isThrown ? Qt::red : Qt::green);
}

void TrackSegment::geometryChanged(const QRectF &, const QRectF &)
{
//    m_top =  m_margin + (m_lineHeight * (m_totalTracks - 1));
    m_top =  m_margin;
}

void TrackSegment::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    if(p == m_lastMousePress && this->contains(m_lastMousePress))
    {
    }
}

void TrackSegment::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePress = event->pos();
}
