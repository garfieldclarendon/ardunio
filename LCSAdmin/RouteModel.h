#ifndef ROUTEMODEL_H
#define ROUTEMODEL_H
#include "UDPMessage.h"
#include "EntityModel.h"

class JSonModel;

class RouteModel : public EntityModel
{
    Q_OBJECT

public:
    RouteModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

signals:

public slots:
    void routeChanged(int routeID, bool isActive, bool isLocked, bool canLock);
    int getRouteRow(int routeID);

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
};

#endif // ROUTEMODEL_H
