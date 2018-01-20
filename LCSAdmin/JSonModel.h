#ifndef METAMODEL_H
#define METAMODEL_H

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonDocument>
#include "Entity.h"

class JSonModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(bool modelChanged READ getModelChanged NOTIFY modelChanged)

public:
    explicit JSonModel(const QJsonDocument &jsonDoc = QJsonDocument(), QObject *parent = nullptr);

    void setJson(const QJsonDocument &jsonDoc, bool emitReset);

    bool getModelChanged(void);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Entity getEntity(int row) const;
    void setEntity(int row, const Entity entity, bool emitSignal);
    QList<int> getModifiedRowIndexes(void) const { return m_modifiedRows; }
    QList<int> getAddedRowIndexes(void) const { return m_addedRows; }
    QList<QJsonObject> getDeletedIndexs(void) const { return m_deletedRows; }
    void clearModifications(void) { m_modifiedRows.clear(); m_addedRows.clear(); m_deletedRows.clear(); }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant data(int row, const QString &key, int role = Qt::DisplayRole) const;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setData(int row, const QString &key, const QVariant &value, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

signals:
    void modelChanged(void);

private:
    QJsonArray m_jsonArray;
    QList<int> m_modifiedRows;
    QList<int> m_addedRows;
    QList<QJsonObject> m_deletedRows;
    mutable int m_columnCount;
};

#endif // METAMODEL_H
