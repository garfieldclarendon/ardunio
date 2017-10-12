#include <QJsonObject>

#include "JSonModel.h"

JSonModel::JSonModel(const QJsonDocument &jsonDoc, QObject *parent)
    : QAbstractTableModel(parent)
{
    setJson(jsonDoc, true);
}

void JSonModel::setJson(const QJsonDocument &jsonDoc, bool emitReset)
{
    if(jsonDoc.isArray())
    {
        if(emitReset)
            beginResetModel();
        m_jsonArray = jsonDoc.array();
        if(emitReset)
            endResetModel();
    }
}

QVariant JSonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant ret;

    if(role == Qt::EditRole || role == Qt::DisplayRole)
    {
        if(orientation == Qt::Vertical)
        {
            ret = section + 1;
        }
        else if(m_jsonArray.count() > 0)
        {
            QJsonObject obj = m_jsonArray[0].toObject();
            QStringList keys = obj.keys();
            ret = keys.value(section);
        }
    }
    else
    {
        QAbstractTableModel::headerData(section, orientation, role);
    }

    return ret;
}

bool JSonModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}


int JSonModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_jsonArray.count();
}

int JSonModel::columnCount(const QModelIndex &parent) const
{
    int ret = 0;
    if (parent.isValid())
        return 0;

    if(rowCount() > 0)
       ret = m_jsonArray[0].toObject().size();
    return ret;
}

QVariant JSonModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if (!index.isValid())
        return QVariant();

    QJsonObject obj = m_jsonArray[index.row()].toObject();
    QString key(obj.keys().value(index.column()));

    if(key.isEmpty() == false)
        ret = data(index.row(), key, role);

    return ret;
}

QVariant JSonModel::data(int row, const QString &key, int role) const
{
    QVariant ret;

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if(row < m_jsonArray.size())
        {
            QJsonObject obj(m_jsonArray[row].toObject());
            ret = obj.value(key).toVariant();
        }
    }

    return ret;
}

bool JSonModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && data(index, role) != value)
    {
        QJsonObject obj(m_jsonArray[index.row()].toObject());
        if(obj.isEmpty() == false)
        {
            QString key(obj.keys().value(index.row()));
            obj[key] = QJsonValue::fromVariant(value);
            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }
    }
    return false;
}

bool JSonModel::setData(int row, const QString &key, const QVariant &value, int role)
{
    if (role == Qt::EditRole && data(row, key, role) != value)
    {
        QJsonObject obj(m_jsonArray[row].toObject());
        if(obj.isEmpty() == false)
        {
            obj[key] = QJsonValue::fromVariant(value);
            m_jsonArray[row] = obj;
            QModelIndex index = this->index(row, obj.keys().indexOf(key));
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

Qt::ItemFlags JSonModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable; // FIXME: Implement me!
}

bool JSonModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();

    return true;
}

bool JSonModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}

bool JSonModel::removeRows(int row, int count, const QModelIndex &parent)
{
    bool ret = false;
    if(row + count - 1 < m_jsonArray.size())
    {
        beginRemoveRows(parent, row, row + count - 1);
        for(int x = 0; x < count; x++)
        {
            m_jsonArray.removeAt(row + x);
        }
        endRemoveRows();
        ret = true;
    }
    return ret;
}

bool JSonModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}
