#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H
#include "UDPMessage.h"
#include "GlobalDefs.h"
#include "EntityModel.h"

class JSonModel;

class DeviceModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int controllerModuleID READ getControllerModuleID WRITE setControllerModuleID NOTIFY controllerIDChanged)
    Q_PROPERTY(int controllerID READ getControllerID WRITE setControllerID NOTIFY controllerIDChanged)
    Q_PROPERTY(int deviceClass READ getClass() WRITE setClass NOTIFY classChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    DeviceModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    void setControllerModuleID(int value);
    int getControllerModuleID(void) const { return m_controllerModuleID; }

    void setControllerID(int value);
    int getControllerID(void) const { return m_controllerID; }

    void setClass(int value);
    int getClass(void) const { return m_class; }

    QString filterText(void) const { return m_filterText; }
    void setFilterText(const QString &filterText);

signals:
    void controllerIDChanged(void);
    void classChanged(void);
    void filterTextChanged(void);

public slots:
    void deviceChanged(int deviceID, int status);
    int getDeviceRow(int deviceID);
    int getDeviceClass(int deviceID);
    Entity createNewDevice(const QString &name, const QString &description, DeviceClassEnum deviceClass, bool createExtra);
    void loadData() override;

    // QAbstractItemModel interface
public:

protected slots:
    void apiReady(void);

    // QAbstractProxyModel interface
public:

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    void createEmptyObject(QJsonObject &obj) override;
    int m_controllerModuleID;
    int m_controllerID;
    DeviceClassEnum m_class;
    QString m_filterText;
};

#endif // DEVICEMODEL_H
