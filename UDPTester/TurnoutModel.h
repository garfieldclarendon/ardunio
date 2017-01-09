#ifndef TURNOUTMODEL_H
#define TURNOUTMODEL_H

#include <QSortFilterProxyModel>
#include "UDPMessage.h"

class QSqlTableModel;

class TurnoutModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int controllerID READ getControllerID WRITE setControllerID NOTIFY controllerIDChanged)

public:
    TurnoutModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const;

    void setControllerID(int value);
    int getControllerID(void) const { return m_controllerID; }

signals:
    void controllerIDChanged(void);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent= QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

protected slots:
    void onNewMessage(const UDPMessage &message);

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    void initArrays(void);
    int m_controllerID;
    QSqlTableModel *m_tableModel;
    QMap<int, QString> m_statusMap;

};

#endif // TURNOUTMODEL_H
