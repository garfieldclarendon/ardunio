#ifndef CONTROLLERMODEL_H
#define CONTROLLERMODEL_H
#include "UDPMessage.h"
#include "GlobalDefs.h"
#include "EntityModel.h"

class JSonModel;

class ControllerModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int controllerClass READ getClass() WRITE setClass NOTIFY classChanged)

public:
    ControllerModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    void setClass(int value);
    int getClass(void) const { return m_class; }

signals:
    void classChanged(void);
    void newController(int newSerialNumber);

public slots:
    void controllerChanged(int serialNumber, ControllerStatusEnum status, quint64 pingLength = 0);
    int getControllerRow(int controllerID);
    int getControllerClass(int controllerID);
    void setControllerID(int value);

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
    ControllerClassEnum m_class;

    // EntityModel interface
public slots:
};

#endif // CONTROLLERMODEL_H
