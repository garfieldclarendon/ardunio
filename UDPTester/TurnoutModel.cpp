#include <QSqlTableModel>
#include <QSqlError>

#include "TurnoutModel.h"
#include "GlobalDefs.h"
#include "JSonModel.h"
#include "API.h"

DeviceModel::DeviceModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_controllerModuleID(0), m_controllerID(0)
{
    connect(API::instance(), SIGNAL(apiReady()), this, SLOT(apiReady()));
    QJsonDocument jsonDoc;
    if(API::instance()->getApiReady())
    {
        QString json = API::instance()->getDeviceList();
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
    }
    m_tableModel = new JSonModel(jsonDoc, this);
    setSourceModel(m_tableModel);
    connect(API::instance(), SIGNAL(deviceChanged(int,int)), this, SLOT(deviceChanged(int,int)));
}

QHash<int, QByteArray> DeviceModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("deviceID");
    roleNames[Qt::UserRole + 1] = QByteArray("deviceName");
    roleNames[Qt::UserRole + 2] = QByteArray("deviceDescription");
    roleNames[Qt::UserRole + 3] = QByteArray("port");
    roleNames[Qt::UserRole + 4] = QByteArray("moduleIndex");
    roleNames[Qt::UserRole + 5] = QByteArray("moduleClass");
    roleNames[Qt::UserRole + 6] = QByteArray("serialNumber");
    roleNames[Qt::UserRole + 7] = QByteArray("deviceState");
    roleNames[Qt::UserRole + 8] = QByteArray("controllerID");

    return roleNames;
}


void DeviceModel::setControllerModuleID(int value)
{
    if(m_controllerModuleID != value)
    {
        m_controllerModuleID = value;
        emit controllerIDChanged();
        invalidateFilter();
    }
}

void DeviceModel::setControllerID(int value)
{
    if(m_controllerID != value)
    {
        m_controllerID = value;
        emit controllerIDChanged();
        invalidateFilter();
    }
}

void DeviceModel::setClass(int value)
{
    if(m_class != (ClassEnum)value)
    {
        m_class = (ClassEnum)value;
        emit classChanged();
        invalidateFilter();
    }
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    QVariant v;
    QModelIndex i(mapToSource(index));
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

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

bool DeviceModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    bool ret = true;
    if(m_controllerModuleID > 0)
    {
        if(m_tableModel->data(source_row, "controllerModuleID", Qt::EditRole).toInt() != m_controllerModuleID)
            ret = false;
    }
    else if(m_controllerID > 0)
    {
        if(m_tableModel->data(source_row, "controllerID", Qt::EditRole).toInt() != m_controllerID)
            ret = false;
    }
    else if(m_class != ClassUnknown)
    {
        if(m_tableModel->data(source_row, "moduleClass", Qt::EditRole).toInt() != (int)m_class)
            ret = false;
    }
    return ret;
}

int DeviceModel::columnCount(const QModelIndex &parent) const
{
    return m_tableModel->columnCount(parent);
}

QModelIndex DeviceModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if(proxyIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapToSource(proxyIndex);
    else
        return QModelIndex(proxyIndex);
}

QModelIndex DeviceModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if(sourceIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapFromSource(sourceIndex);
    else
        return QModelIndex(sourceIndex);
}

void DeviceModel::apiReady()
{
    QJsonDocument jsonDoc;
    if(API::instance()->getApiReady())
    {
        QString json = API::instance()->getDeviceList();
        jsonDoc = QJsonDocument::fromJson(json.toLatin1());
        beginResetModel();
        m_tableModel->setJson(jsonDoc, false);
        endResetModel();
    }
}

void DeviceModel::deviceChanged(int deviceID, int status)
{
    for(int x = 0; x < m_tableModel->rowCount(); x++)
    {
        int d = m_tableModel->data(x, "deviceID", Qt::EditRole).toInt();
        if(d == deviceID)
        {
            m_tableModel->setData(x, "deviceState", status);
            break;
        }
    }
}
