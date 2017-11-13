#ifndef SIGNALASPECTMODEL_H
#define SIGNALASPECTMODEL_H

#include "../ControlServer/UDPMessage.h"
#include "EntityModel.h"

class JSonModel;

class SignalAspectModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int deviceID READ getdeviceID WRITE setdeviceID NOTIFY deviceIDChanged)

public:
    SignalAspectModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    void setdeviceID(int value);
    int getdeviceID(void) const { return m_deviceID; }

signals:
    void deviceIDChanged(void);

public slots:

    // QAbstractItemModel interface
public:
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected slots:
    void apiReady(void);

    // QAbstractProxyModel interface
public:

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    void createEmptyObject(QJsonObject &obj) override;

    int m_deviceID;
};

#endif // SIGNALASPECTMODEL_H
