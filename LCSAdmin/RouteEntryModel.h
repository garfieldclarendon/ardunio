#ifndef ROUTEENTRYMODEL_H
#define ROUTEENTRYMODEL_H
#include "EntityModel.h"

class JSonModel;

class RouteEntryModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int routeID READ getRouteID WRITE setRouteID NOTIFY routeIDChanged)

public:
    RouteEntryModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;
    int getRouteID(void) const { return m_routeID; }
    void setRouteID(int value);

signals:
    void routeIDChanged(void);

public slots:
    int getRouteEntryRow(int routeEntryID);

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
    int m_routeID;
};

#endif // ROUTEENTRYMODEL_H
