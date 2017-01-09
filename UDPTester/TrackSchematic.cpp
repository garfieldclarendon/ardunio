#include "TrackSchematic.h"

TrackSchematic::TrackSchematic(QWidget *parent) : QWidget(parent)
{   
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void TrackSchematic::addTrackSection(QWidget *section)
{
    section->setParent(this);
    update();
}

void TrackSchematic::resizeEvent(QResizeEvent *)
{
    QWidget *section = qobject_cast<QWidget *>(children().first());
    // center the section
    int y = (height() / 2) - (section->sizeHint().height() / 2);
    int x = (width() / 2) - (section->sizeHint().width() / 2);

    section->setGeometry(x, y, section->sizeHint().width(), section->sizeHint().height());
}

