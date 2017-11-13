#ifndef SIGNALCONDITIONMODEL_H
#define SIGNALCONDITIONMODEL_H

#include "../ControlServer/UDPMessage.h"
#include "EntityModel.h"

class JSonModel;

class SignalConditionModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int aspectID READ getAspectID WRITE setAspectID NOTIFY aspectIDChanged)

public:
    SignalConditionModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    void setAspectID(int value);
    int getAspectID(void) const { return m_aspectID; }

signals:
    void aspectIDChanged(void);

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

    int m_aspectID;
};

#endif // SIGNALCONDITIONMODEL_H
