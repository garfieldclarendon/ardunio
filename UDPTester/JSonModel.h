#ifndef METAMODEL_H
#define METAMODEL_H

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonDocument>

class JSonModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit JSonModel(const QJsonDocument &jsonDoc = QJsonDocument(), QObject *parent = nullptr);

    void setJson(const QJsonDocument &jsonDoc, bool emitReset);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

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

private:
    QJsonArray m_jsonArray;
};

#endif // METAMODEL_H
