#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <QObject>
#include "GlobalDefs.h"

class DeviceHandler : public QObject
{
    Q_OBJECT
public:
    explicit DeviceHandler(DeviceClassEnum classCode, QObject *parent = 0);

    DeviceClassEnum getClassCode(void) const { return m_classCode; }

signals:

public slots:

private:
    DeviceClassEnum m_classCode;
};

#endif // DEVICEHANDLER_H
