#ifndef TURNOUTMODEL_H
#define TURNOUTMODEL_H

#include <QSortFilterProxyModel>
#include "UDPMessage.h"

class JSonModel;

class DeviceModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int controllerModuleID READ getControllerModuleID WRITE setControllerModuleID NOTIFY controllerIDChanged)
    Q_PROPERTY(int controllerID READ getControllerID WRITE setControllerID NOTIFY controllerIDChanged)
    Q_PROPERTY(int deviceClass READ getClass() WRITE setClass NOTIFY classChanged)

public:
    DeviceModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const;

    void setControllerModuleID(int value);
    int getControllerModuleID(void) const { return m_controllerModuleID; }

    void setControllerID(int value);
    int getControllerID(void) const { return m_controllerID; }

    void setClass(int value);
    int getClass(void) const { return m_class; }

signals:
    void controllerIDChanged(void);
    void classChanged(void);

public slots:
    void deviceChanged(int deviceID, int status);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected slots:
    void apiReady(void);

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_controllerModuleID;
    int m_controllerID;
    ClassEnum m_class;

    JSonModel *m_tableModel;

};

#endif // TURNOUTMODEL_H
