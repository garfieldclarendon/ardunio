#include <QSqlTableModel>
#include <QTimer>
#include <QDateTime>
#include <QSqlError>

#include "ControllerModel.h"
#include "Database.h"
#include "MessageBroadcaster.h"
#include "TcpServer.h"

ControllerModel::ControllerModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_tableModel(NULL), m_filterByOnline(-1)
{
    Database db;
    m_tableModel = new QSqlTableModel(this, db.getDatabase());
    m_tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_tableModel->setTable("controller");
    m_tableModel->select();
    initArrays();

    this->setSourceModel(m_tableModel);

    QTimer::singleShot(1000, this, SLOT(timerProc()));
//QTimer::singleShot(5000, this, SLOT(tmpTimerSlot()));
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(onNewMessage(UDPMessage)));
    connect(TcpServer::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(onNewMessage(UDPMessage)));
}

QHash<int, QByteArray> ControllerModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("id");
    roleNames[Qt::UserRole + 1] = QByteArray("controllerName");
    roleNames[Qt::UserRole + 2] = QByteArray("controllerClass");
    roleNames[Qt::UserRole + 3] = QByteArray("controllerDescription");
    roleNames[Qt::UserRole + 4] = QByteArray("serialNumber");
    roleNames[Qt::UserRole + 5] = QByteArray("currentStatus");
    roleNames[Qt::UserRole + 6] = QByteArray("version");

    return roleNames;
}

int ControllerModel::getControllerID(int row) const
{
    QModelIndex index = this->index(row, m_tableModel->fieldIndex("id"));
    return QSortFilterProxyModel::data(index, Qt::EditRole).toInt();
}

QVariant ControllerModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    int controllerID = getControllerID(index.row());
    if(role >= Qt::UserRole)
    {
        QModelIndex i;
        int col = role - Qt::UserRole;
        switch(col)
        {
        case 0:
            i = this->index(index.row(), m_tableModel->fieldIndex("id"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 1:
            i = this->index(index.row(), m_tableModel->fieldIndex("controllerName"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 2:
            i = this->index(index.row(), m_tableModel->fieldIndex("controllerClass"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 3:
            i = this->index(index.row(), m_tableModel->fieldIndex("controllerDescription"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 4:
            i = this->index(index.row(), m_tableModel->fieldIndex("serialNumber"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 5:
            v = m_onlineStatusMap.value(controllerID);
            break;
        case 6:
            v = m_versionMap.value(controllerID);
            break;
        default:
            break;
        }
    }
    else if(index.column() == 6 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        v = m_onlineStatusMap.value(controllerID);
    }
    else if(index.column() == 7 && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        v = m_versionMap.value(controllerID);
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

void ControllerModel::onNewMessage(const UDPMessage &message)
{
    int controllerID = message.getControllerID();
    if(controllerID > 0)
    {
        int rows = rowCount() - 1;
        int cols = columnCount() - 1;
        m_versionMap[controllerID] = QString("%1").arg(message.getMessageVersion());
        m_onlineStatusMap[controllerID] = "Online";
        m_timeoutMap[controllerID] = QDateTime::currentDateTime().toTime_t();

        QModelIndex start = this->index(0, cols);
        QModelIndex end = this->index(rows, cols);
        emit dataChanged(start, end);
    }
}

void ControllerModel::timerProc()
{
    QMapIterator<int, long> i(m_timeoutMap);
    while (i.hasNext())
    {
        i.next();
        if(QDateTime::currentDateTime().toTime_t() - i.value() > 15)
        {
            if(m_onlineStatusMap[i.key()] != "?")
            {
                m_onlineStatusMap[i.key()] = "Offline";

                int rows = rowCount() - 1;
                int cols = columnCount() - 1;
                QModelIndex start = this->index(0, cols);
                QModelIndex end = this->index(rows, cols);
                emit dataChanged(start, end);
            }
        }
    }
    QTimer::singleShot(1000, this, SLOT(timerProc()));
}

QVariant ControllerModel::getData(int row, const QString &fieldName) const
{
    QModelIndex index = this->index(row, m_tableModel->fieldIndex(fieldName));
    return this->data(index, Qt::EditRole);
}

void ControllerModel::setData(int row, const QString &fieldName, const QVariant &value)
{
    QModelIndex index = this->index(row, m_tableModel->fieldIndex(fieldName));
    QModelIndex i = mapToSource(index);

    m_tableModel->setData(i, value, Qt::EditRole);
//    m_tableModel->submitAll();
    QString errorText = m_tableModel->lastError().text();
    qDebug(errorText.toLatin1());
}

void ControllerModel::setNewSerialNumber(int controllerID, const QString &serialNumber)
{
    QModelIndex index;
    int newSerialNumber = serialNumber.toInt();
    for(int x = 0; x < m_tableModel->rowCount(); x++)
    {
        index = m_tableModel->index(x, m_tableModel->fieldIndex("id"));
        int id = m_tableModel->data(index, Qt::EditRole).toInt();

        if(id == controllerID)
        {
            index = m_tableModel->index(x, m_tableModel->fieldIndex("serialNumber"));
            m_tableModel->setData(index, newSerialNumber);
            m_tableModel->submitAll();
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
    m_tableModel->submitAll();
    QString errorText = m_tableModel->lastError().text();
    qDebug(errorText.toLatin1());
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
        QModelIndex i = m_tableModel->index(source_row, m_tableModel->fieldIndex("currentStatus"));
        bool online = m_tableModel->data(i, Qt::EditRole).toBool();
        if(m_filterByOnline == 0 && online)
            ret = false;
        else if(m_filterByOnline == 1 && !online)
            ret = false;
    }
    else if(m_textFilter.length() > 0)
    {
        QString id, name, desc, serial;
        QModelIndex i = m_tableModel->index(source_row, m_tableModel->fieldIndex("id"));
        id = m_tableModel->data(i, Qt::EditRole).toString();

        i = m_tableModel->index(source_row, m_tableModel->fieldIndex("controllerName"));
        name = m_tableModel->data(i, Qt::EditRole).toString();

        i = m_tableModel->index(source_row, m_tableModel->fieldIndex("controllerDescription"));
        desc = m_tableModel->data(i, Qt::EditRole).toString();

        i = m_tableModel->index(source_row, m_tableModel->fieldIndex("serialNumber"));
        serial = m_tableModel->data(i, Qt::EditRole).toString();

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
    m_versionMap.clear();
    m_onlineStatusMap.clear();

    for(int x = 0; x < m_tableModel->rowCount(); x++)
    {
        QModelIndex i = m_tableModel->index(x, m_tableModel->fieldIndex("id"));
        int controllerID = m_tableModel->data(i, Qt::EditRole).toInt();

        m_versionMap[controllerID] = "?";
        m_onlineStatusMap[controllerID] = "?";
        m_timeoutMap[controllerID] = QDateTime::currentDateTime().toTime_t();
    }
}

void ControllerModel::handleNewControllerMessage(const UDPMessage &message)
{
    Database db;
    QString serialNumber = QString("%1").arg(message.getLValue());
    int deviceID = db.getControllerID(message.getLValue());

    if(deviceID == -1)
    {
        emit newController(serialNumber, message.getByteValue1());
    }
}

QModelIndex ControllerModel::parent(const QModelIndex &child) const
{
    if(child.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::parent(child);
    else
        return QModelIndex();
}
