#ifndef ENTITYMODEL_H
#define ENTITYMODEL_H

#include <QSortFilterProxyModel>
#include <QJsonObject>

#include "Entity.h"

class JSonModel;

class EntityModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ getRowCount NOTIFY rowCountChanged)

public:
    EntityModel(const QString &entityName, QObject *parent = NULL);
    QString getEntityName(void) const { return m_entityName; }

signals:
    void rowCountChanged(void);
    void error(int row, const QString &errorText);

public slots:
    virtual Entity getEntity(int row);
    void setEntity(int row, const Entity &entity);
    void deleteRow(int row);
    void moveUp(int row);
    void moveDown(int row);
    bool save(void);
    int getRowCount(void) const;
    QVariant data(int row, const QString &key) const;
    virtual void loadData(void) = 0;

    // QAbstractItemModel interface
public:
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected slots:

    // QAbstractProxyModel interface
public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    // QSortFilterProxyModel interface
protected:
    virtual void createEmptyObject(QJsonObject &obj);

    JSonModel *m_jsonModel;
    QString m_entityName;
};

#endif // ENTITYMODEL_H
