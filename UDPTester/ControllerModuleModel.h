#ifndef CONTROLLERMODULEMODEL_H
#define CONTROLLERMODULEMODEL_H

#include <QSortFilterProxyModel>
#include "GlobalDefs.h"

class QSqlTableModel;

class ControllerModuleModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int controllerID READ getControllerID WRITE setControllerID NOTIFY controllerIDChanged)
    Q_PROPERTY(int controllerClass READ getControllerClass WRITE setControllerClass NOTIFY controllerClassChanged)
    Q_PROPERTY(bool allowNewModule READ getAllowNewModule NOTIFY allowNewModuleChanged)

public:
    ControllerModuleModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const;

    void setControllerID(int value);
    int getControllerID(void) const { return m_controllerClass; }
    void setControllerClass(int value);
    int getControllerClass(void) const { return m_controllerClass; }

public slots:
    int addNew(const QString &moduleName, int moduleIndex, int moduleClass);
    int createDeviceEntry(int moduleID, int moduleIndex, const QString &deviceName);

signals:
    void controllerIDChanged(void);
    void controllerClassChanged(void);
    void allowNewModuleChanged(void);

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent= QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    bool getAllowNewModule(void) const;
    void setAllowNewModule(bool value);

protected slots:
    int getModuleClass(int row) const;
    void timerProc(void);

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    // QSortFilterProxyModel interface
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int m_controllerID;
    int m_controllerClass;
    QSqlTableModel *m_tableModel;
};

#endif // CONTROLLERMODULEMODEL_H
