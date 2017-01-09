#ifndef PANELMODULEMODEL_H
#define PANELMODULEMODEL_H

#include <QSortFilterProxyModel>

class QSqlTableModel;

class PanelModuleModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int controllerID READ getControllerID WRITE setControllerID NOTIFY controllerIDChanged)

public:
    PanelModuleModel(QObject *parent = NULL);
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
    int rowCount(const QModelIndex &parent= QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_controllerID;
    QSqlTableModel *m_tableModel;
};

#endif // PANELMODULEMODEL_H
