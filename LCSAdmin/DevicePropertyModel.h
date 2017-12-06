#ifndef DEVICEPROPERTYMODEL_H
#define DEVICEPROPERTYMODEL_H
#include "../LCSServer/UDPMessage.h"
#include "EntityModel.h"

class JSonModel;

class DevicePropertyModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int deviceID READ getDeviceID WRITE setDeviceID NOTIFY deviceIDChanged)
    Q_PROPERTY(int deviceClass READ getClass() WRITE setClass NOTIFY classChanged)

public:
    DevicePropertyModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    void setDeviceID(int value);
    int getDeviceID(void) const { return m_deviceID; }

    void setClass(int value);
    int getClass(void) const { return m_class; }

signals:
    void deviceIDChanged(void);
    void classChanged(void);

public slots:

    // QAbstractItemModel interface
public:

protected slots:
    void apiReady(void);

    // QAbstractProxyModel interface
public:

private:
    void createEmptyObject(QJsonObject &obj) override;
    void checkKeys(void);
    void checkTurnoutKeys(void);
    void checkPanelInputKeys(void);
    void checkPanelOutputKeys(void);

    int m_deviceID;
    DeviceClassEnum m_class;
};

#endif // DEVICEMODEL_H
