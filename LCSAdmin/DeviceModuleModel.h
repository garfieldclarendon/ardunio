#ifndef DEVICEMODULEMODEL_H
#define DEVICEMODULEMODEL_H
#include "UDPMessage.h"
#include "GlobalDefs.h"
#include "EntityModel.h"

class JSonModel;

class DeviceModuleModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int deviceID READ getDeviceID WRITE setDeviceID NOTIFY deviceIDChanged)
    Q_PROPERTY(int moduleID READ getModuleID() WRITE setModuleID NOTIFY moduleIDChanged)
    Q_PROPERTY(int controllerID READ getControllerID() WRITE setControllerID NOTIFY controllerIDChanged)

public:
    DeviceModuleModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    void setDeviceID(int value);
    int getDeviceID(void) const { return m_deviceID; }

    void setModuleID(int value);
    int getModuleID(void) const { return m_moduleID; }

    void setControllerID(int value);
    int getControllerID(void) const { return m_controllerID; }

signals:
    void deviceIDChanged(void);
    void moduleIDChanged(void);
    void controllerIDChanged(void);

    // QAbstractItemModel interface
public slots:
    void loadData() override;

protected slots:
    void apiReady(void);

    // QAbstractProxyModel interface
public:

private:
    void createEmptyObject(QJsonObject &obj) override;

    int m_deviceID;
    int m_moduleID;
    int m_controllerID;
};

#endif // DEVICEMODULEMODEL_H
