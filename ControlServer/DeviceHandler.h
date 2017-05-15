#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <QObject>
#include "GlobalDefs.h"

class DeviceHandler : public QObject
{
    Q_OBJECT
public:
    explicit DeviceHandler(ClassEnum classCode, QObject *parent = 0);

    ClassEnum getClassCode(void) const { return m_classCode; }

signals:

public slots:

private:
    ClassEnum m_classCode;
};

#endif // DEVICEHANDLER_H
