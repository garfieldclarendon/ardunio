#ifndef CONTROLLERMODULEMODEL_H
#define CONTROLLERMODULEMODEL_H

#include "EntityModel.h"

class JSonModel;

class ControllerModuleModel : public EntityModel
{
    Q_OBJECT
    Q_PROPERTY(int controllerID READ getControllerlID WRITE setControllerID NOTIFY controllerIDChanged)

public:
    ControllerModuleModel(QObject *parent = NULL);
    QHash<int, QByteArray> roleNames(void) const override;

    void setControllerID(int value);
    int getControllerlID(void) const { return m_controllerID; }
    void setControllerModuleID(int value);

signals:
    void controllerIDChanged(void);

public slots:
    void loadData() override;

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

    int m_controllerID;
    int m_controllerModuleID;
};

#endif // CONTROLLERMODULEMODEL_H
