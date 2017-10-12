#include <QTimer>
#include <QDateTime>

#include "ControllerModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

ControllerModel::ControllerModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_tableModel(NULL), m_filterByOnline(-1)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    connect(API::instance(), SIGNAL(controllerChanged(int,ControllerStatus)), this, SLOT(controllerChanged(int,ControllerStatus)));
    QJsonDocument jsonDoc;
    if(API::instance()->getApiReady())
    {
        QString json = API::instance()->getControllerList();
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
    }
    m_tableModel = new JSonModel(jsonDoc, this);
    setSourceModel(m_tableModel);
}

QHash<int, QByteArray> ControllerModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("controllerID");
    roleNames[Qt::UserRole + 1] = QByteArray("controllerName");
    roleNames[Qt::UserRole + 2] = QByteArray("controllerClass");
    roleNames[Qt::UserRole + 3] = QByteArray("controllerDescription");
    roleNames[Qt::UserRole + 4] = QByteArray("serialNumber");
    roleNames[Qt::UserRole + 5] = QByteArray("status");
    roleNames[Qt::UserRole + 6] = QByteArray("version");

    return roleNames;
}

int ControllerModel::getControllerID(int row) const
{
    QModelIndex index = this->index(row, 0);
    QModelIndex i = mapToSource(index);
    return m_tableModel->data(i.row(), "controllerID", Qt::EditRole).toInt();
}

QVariant ControllerModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    QModelIndex i = mapToSource(index);
    if(role >= Qt::UserRole)
    {
        QHash<int, QByteArray> roles(roleNames());
        v = m_tableModel->data(i.row(), QString(roles[role]), Qt::EditRole);
    }
    else
    {
        v = QSortFilterProxyModel::data(index, role);
    }

    return v;
}

bool ControllerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

QVariant ControllerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QSortFilterProxyModel::headerData(section, orientation, role);
}

void ControllerModel::timerProc()
{
    QMapIterator<int, long> i(m_timeoutMap);
    while (i.hasNext())
    {
        i.next();
        if(QDateTime::currentDateTime().toTime_t() - i.value() > HEARTBEAT_INTERVAL * 2)
        {
            for(int x = 0; x < m_tableModel->rowCount(); x++)
            {
                if(i.key() == m_tableModel->data(x, "controllerID").toInt())
                {
                    m_tableModel->setData(x, "status", "Offline");
                    break;
                }
            }
        }
    }
    QTimer::singleShot(1000, this, SLOT(timerProc()));
}

void ControllerModel::apiReady()
{
    QJsonDocument jsonDoc;
    if(API::instance()->getApiReady())
    {
        QString json = API::instance()->getControllerList();
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_tableModel->setJson(jsonDoc, false);
        initArrays();
        endResetModel();
    }
}

QVariant ControllerModel::getData(int row, const QString &fieldName) const
{
    QModelIndex index = this->index(row, 0);
    QModelIndex i = mapToSource(index);
    return m_tableModel->data(i.row(), fieldName);
}

void ControllerModel::setData(int row, const QString &fieldName, const QVariant &value)
{
    QModelIndex index = this->index(row, 0);
    QModelIndex i = mapToSource(index);

    m_tableModel->setData(i.row(), fieldName, value);
//    m_tableModel->submitAll();
}

void ControllerModel::setNewSerialNumber(int controllerID, const QString &serialNumber)
{
    QModelIndex index;
    int newSerialNumber = serialNumber.toInt();
    for(int x = 0; x < m_tableModel->rowCount(); x++)
    {
        int id = m_tableModel->data(x, "controllerID", Qt::EditRole).toInt();

        if(id == controllerID)
        {
            m_tableModel->setData(x, "serialNumber", newSerialNumber);
            break;
        }
    }
}

void ControllerModel::controllerChanged(int serialNumber, ControllerStatus status)
{
    for(int x = 0; x < m_tableModel->rowCount(); x++)
    {
        int s = m_tableModel->data(x, "serialNumber", Qt::EditRole).toInt();
        if(s == serialNumber)
        {
            m_tableModel->setData(x, "status", status);
            break;
        }
    }
}

int ControllerModel::addNew()
{
    int row = rowCount();
    if(insertRow(row))
        return row;
    return -1;
}

void ControllerModel::deleteRow(int row)
{
    removeRow(row);
}

void ControllerModel::save()
{
}

void ControllerModel::filterByOnline(bool online)
{
    m_textFilter.clear();
    m_filterByOnline = online;
    invalidateFilter();
}

void ControllerModel::filterByText(const QString &text)
{
    qDebug(QString("filterByText: %1").arg(text).toLatin1());
    m_filterByOnline = -1;
    m_textFilter = text;

    invalidateFilter();
}

void ControllerModel::clearFilter()
{
    m_textFilter.clear();
    m_filterByOnline = -1;
    invalidateFilter();
}

void ControllerModel::tmpTimerSlot()
{
    emit newController("1234", 1);
}

QModelIndex ControllerModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex i;
    if(column < m_tableModel->columnCount())
        i = QSortFilterProxyModel::index(row, column, parent);
    else if(column < m_tableModel->columnCount() + 2)
        i = createIndex(row, column);

    return i;
}

int ControllerModel::columnCount(const QModelIndex &parent) const
{
    return m_tableModel->columnCount(parent) + 2;
}

QModelIndex ControllerModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if(proxyIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapToSource(proxyIndex);
    else
        return QModelIndex(proxyIndex);
}

QModelIndex ControllerModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if(sourceIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapFromSource(sourceIndex);
    else
        return QModelIndex(sourceIndex);
}

bool ControllerModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
//    qDebug(QString("filterAcceptsRow: %1 %2").arg(m_textFilter).arg(m_filterByOnline).toLatin1());
    bool ret = true;
    if(m_filterByOnline != -1)
    {
        bool online = m_tableModel->data(source_row, "currentStatus", Qt::EditRole).toBool();
        if(m_filterByOnline == 0 && online)
            ret = false;
        else if(m_filterByOnline == 1 && !online)
            ret = false;
    }
    else if(m_textFilter.length() > 0)
    {
        QString id, name, desc, serial;
        id = m_tableModel->data(source_row, "controllerID", Qt::EditRole).toString();

        name = m_tableModel->data(source_row, "controllerName", Qt::EditRole).toString();

        desc = m_tableModel->data(source_row, "controllerDescription", Qt::EditRole).toString();

        serial = m_tableModel->data(source_row, "serialNumber", Qt::EditRole).toString();

        qDebug(QString("filterAcceptsRow: %1 %2 %3 %4").arg(id).arg(name).arg(desc).arg(serial).toLatin1());
        if(id.startsWith(m_textFilter, Qt::CaseInsensitive) || name.startsWith(m_textFilter, Qt::CaseInsensitive) || desc.startsWith(m_textFilter, Qt::CaseInsensitive) || serial.startsWith(m_textFilter, Qt::CaseInsensitive))
            ret = true;
        else
            ret = false;
    }

    return ret;
}

void ControllerModel::initArrays()
{
    for(int x = 0; x < m_tableModel->rowCount(); x++)
    {
        int controllerID = m_tableModel->data(x, "controllerID", Qt::EditRole).toInt();

        m_timeoutMap[controllerID] = QDateTime::currentDateTime().toTime_t();
    }
}

QModelIndex ControllerModel::parent(const QModelIndex &child) const
{
    if(child.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::parent(child);
    else
        return QModelIndex();
}
