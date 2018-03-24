#include "EntityModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

EntityModel::EntityModel(const QString &entityName, QObject *parent)
    : QSortFilterProxyModel(parent), m_jsonModel(NULL), m_entityName(entityName)
{
    QJsonDocument doc;
    m_jsonModel = new JSonModel(doc, this);
    connect(m_jsonModel, SIGNAL(modelReset()), this, SIGNAL(rowCountChanged()));
    setSourceModel(m_jsonModel);
}

QVariant EntityModel::data(const QModelIndex &index, int role) const
{
    QVariant v;
    QModelIndex i(mapToSource(index));
    if(role >= Qt::UserRole)
    {
        QHash<int, QByteArray> roles(roleNames());
        v = m_jsonModel->data(i.row(), QString(roles[role]), Qt::EditRole);
    }
    else
    {
        v = QSortFilterProxyModel::data(index, role);
    }

    return v;
}

QVariant EntityModel::data(int row, const QString &key) const
{
    QModelIndex i = index(row, 0);
    i = mapToSource(i);
    return m_jsonModel->data(i.row(), key, Qt::DisplayRole);
}

bool EntityModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role >= Qt::UserRole)
    {
        QModelIndex i = mapToSource(index);
        QString key = roleNames().value(role);
        return m_jsonModel->setData(i.row(), key, value);
    }
    else
    {
        return QSortFilterProxyModel::setData(index, value, role);
    }
}

int EntityModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return roleNames().keys().count();
}

void EntityModel::deleteRow(int row)
{
    QModelIndex i = index(row, 0);
    QModelIndex i2;
    i2 = mapToSource(i);
    if(i2.isValid())
        m_jsonModel->removeRow(i2.row());
    rowCountChanged();
}

void EntityModel::moveUp(int row, const QString &fieldName)
{
    int rows = rowCount();
    if(row > 0 && row < rows)
    {
        Entity one = getEntity(row);
        Entity two = getEntity(row - 1);
        QJsonObject obj = one.getObject();
        obj[fieldName] = row - 1;
        one.setObject(obj);
        obj = two.getObject();
        obj[fieldName] = row + 1;
        two.setObject(obj);

        m_jsonModel->setEntity(row - 1, one, false);
        m_jsonModel->setEntity(row, two, false);
        QModelIndex start = index(row, 0);
        QModelIndex end = index(row, columnCount() - 1);
        emit dataChanged(start, end);
        start = index(row-1, 0);
        end = index(row-1, columnCount() - 1);
        emit dataChanged(start, end);
    }
}

void EntityModel::moveDown(int row, const QString &fieldName)
{
    if(row >= 0 && row < rowCount() - 1)
    {
        Entity one = getEntity(row);
        Entity two = getEntity(row + 1);
        QJsonObject obj = one.getObject();
        obj[fieldName] = row + 1;
        one.setObject(obj);
        obj = two.getObject();
        obj[fieldName] = row - 1;
        two.setObject(obj);

        m_jsonModel->setEntity(row + 1, one, false);
        m_jsonModel->setEntity(row, two, false);
        QModelIndex start = index(row, 0);
        QModelIndex end = index(row, columnCount() - 1);
        emit dataChanged(start, end);
        start = index(row+1, 0);
        end = index(row+1, columnCount() - 1);
        emit dataChanged(start, end);
    }
}

bool EntityModel::save()
{
    bool ret = false;
    qDebug(QString("EntityModel::save(): %1").arg(m_entityName).toLatin1());
    if(API::instance()->getApiReady())
    {
        QList<int> rows;
        rows = m_jsonModel->getModifiedRowIndexes();
        for(int x = 0; x < rows.count(); x++)
        {
            Entity entity = getEntity(rows.value(x));
            Entity retEntity = API::instance()->saveEntity(entity);
            qDebug("EntityModel::save()  AFTER API SAVE");
            if(retEntity.hasError())
                emit error(rows.value(x), retEntity.errorText());
            else
                m_jsonModel->setEntity(rows.value(x), retEntity, true);
        }

        rows = m_jsonModel->getAddedRowIndexes();
        for(int x = 0; x < rows.count(); x++)
        {
            Entity entity = getEntity(rows.value(x));
            Entity retEntity = API::instance()->saveEntity(entity, true);
            if(retEntity.hasError())
                emit error(rows.value(x), retEntity.errorText());
            else
                m_jsonModel->setEntity(rows.value(x), retEntity, true);
        }

        QList<QJsonObject> objs = m_jsonModel->getDeletedIndexs();
        for(int x = 0; x < objs.count(); x++)
        {
            Entity entity(objs.value(x));
            entity.setEntityName(m_entityName);
            Entity retEntity = API::instance()->deleteEntity(entity);
            if(retEntity.hasError())
                emit error(rows.value(x), retEntity.errorText());
        }
        m_jsonModel->clearModifications();
    }
    else
    {
        emit error(-1, "Server offline.");
    }
    return ret;
}

int EntityModel::getRowCount() const
{
    int count = rowCount();
    return count;
}

QModelIndex EntityModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if(proxyIndex.column() < m_jsonModel->columnCount())
        return QSortFilterProxyModel::mapToSource(proxyIndex);
    else
        return QModelIndex(proxyIndex);
}

QModelIndex EntityModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if(sourceIndex.column() < m_jsonModel->columnCount())
        return QSortFilterProxyModel::mapFromSource(sourceIndex);
    else
        return QModelIndex(sourceIndex);
}

Entity EntityModel::getEntity(int row)
{
    QModelIndex i = index(row, 0);
    QModelIndex i2 = mapToSource(i);
    Entity entity;
    QJsonObject obj;
    if(row >= 0)
    {
        entity = m_jsonModel->getEntity(i2.row());
        obj = entity.getObject();
    }
    else
    {
        createEmptyObject(obj);
    }

    entity.setEntityName(m_entityName);
    entity.setObject(obj);
    return entity;
}

void EntityModel::setEntity(int row, const Entity &entity)
{
    int r = row;
    if(row >= 0)
    {
        QModelIndex i = index(row, 0);
        QModelIndex i2 = mapToSource(i);
        r = i2.row();
    }

    int count = rowCount();
    m_jsonModel->setEntity(r, entity, true);
    if(count != rowCount())
        rowCountChanged();
}

void EntityModel::createEmptyObject(QJsonObject &obj)
{
    QHash<int, QByteArray> names(roleNames());

    for(int x = 0; x < names.values().count(); x++)
    {
        QString key(names.values().value(x));
        QVariant v;
        obj[key] = QJsonValue::fromVariant(v);
    }
}
