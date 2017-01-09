#ifndef PANELOUTPUTMODEL_H
#define PANELOUTPUTMODEL_H

#include <QSortFilterProxyModel>

class QSqlTableModel;

class PanelOutputModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int panelModuleID READ getPanelModuleID WRITE setPanelModuleID NOTIFY panelModuleIDChanged)

public:
    PanelOutputModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const;

    void setPanelModuleID(int value);
    int getPanelModuleID(void) const { return m_panelModuleID; }

signals:
    void panelModuleIDChanged(void);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_panelModuleID;
    QSqlTableModel *m_tableModel;
};

#endif // PANELOUTPUTMODEL_H
