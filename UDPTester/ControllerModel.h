#ifndef CONTROLLERMODEL_H
#define CONTROLLERMODEL_H

#include <QSortFilterProxyModel>
#include "UDPMessage.h"

class QSqlTableModel;

class ControllerModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ControllerModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const;

signals:
    void newController(const QString &serialNumber, int controllerClass);

public slots:
    int getControllerID(int row) const;
    QVariant getData(int row, const QString &fieldName) const;
    void setData(int row, const QString &fieldName, const QVariant &value);
    void setNewSerialNumber(int controllerID, const QString &serialNumber);

    int addNew(void);
    void deleteRow(int row);
    void save(void);

    void filterByOnline(bool online);
    void filterByText(const QString &text);
    void clearFilter(void);

    void tmpTimerSlot(void);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

protected slots:
    void onNewMessage(const UDPMessage &message);
    void timerProc(void);

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    void initArrays(void);
    void handleNewControllerMessage(const UDPMessage &message);

    QSqlTableModel *m_tableModel;
    QMap<int, long> m_timeoutMap;
    QMap<int, QString> m_onlineStatusMap;
    QMap<int, QString> m_versionMap;
    int m_filterByOnline;
    QString m_textFilter;
};

#endif // CONTROLLERMODEL_H
