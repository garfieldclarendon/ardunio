#include <QPaintEvent>
#include <QPainter>

#include "TrackTrunout.h"

TrackTrunout::TrackTrunout(QWidget *parent)
    : QWidget(parent), turnoutID(2)
{
    currentState = "Unknown";
    brush = Qt::black;
    adjustPoints();
}

QSize TrackTrunout::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize TrackTrunout::sizeHint() const
{
    return QSize(400, 200);
}

void TrackTrunout::paintEvent(QPaintEvent * /*event*/)
{
    adjustPoints();

    QPainter painter(this);
    painter.save();
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int h = height() * .15;
    int y = height() - height()/2 - h;
    painter.setBrush(brush);
    painter.drawPolygon(points, 4);
    painter.setBrush(Qt::black);
    painter.drawRect(7, y, width() - 14, h);
    painter.restore();

    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
    painter.setPen(Qt::black);
    painter.drawText(QRect(3, 3, width() - 1, height() - 1), currentState);
}

void TrackTrunout::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // if brush is set to Qt::black, then the turnout is set to
        // the normal route
//        sendMessage(brush == Qt::black);
        sendMessage(brush == Qt::black);
    }
}

void TrackTrunout::adjustPoints()
{
    int h = height() * .15;
    int w = height() - height()/2 - h;

    points[0] = QPoint(50, w);
    points[1] = QPoint(50 + .5*h, w + h);
    points[2] = QPoint(width() - 7, h + 5);
    points[3] = QPoint(width() - 7, 5);
}

void TrackTrunout::sendMessage(bool toDiverging)
{
}

