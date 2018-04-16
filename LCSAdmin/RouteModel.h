#ifndef ROUTEMODEL_H
#define ROUTEMODEL_H
#include "UDPMessage.h"
#include "EntityModel.h"

class JSonModel;

class RouteModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(int deviceID READ deviceID WRITE setDeviceID NOTIFY deviceIDChanged)

public:
    RouteModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    int deviceID(void) const { return m_deviceID; }
    void setDeviceID(int value);

    QString filterText(void) const { return m_filterText; }
    void setFilterText(const QString &filterText);

signals:
    void filterTextChanged(void);
    void deviceIDChanged(void);

public slots:
    void routeChanged(int routeID, bool isActive, bool isLocked, bool canLock);
    int getRouteRow(int routeID);
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
    QString m_filterText;
    int m_deviceID;
};

#endif // ROUTEMODEL_H
