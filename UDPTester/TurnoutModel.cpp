#include <QSqlTableModel>

#include "TurnoutModel.h"
#include "Database.h"
#include "GlobalDefs.h"
#include "MessageBroadcaster.h"
#include "TcpServer.h"

TurnoutModel::TurnoutModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_controllerID(0)
{
    Database db;
    m_tableModel = new QSqlTableModel(this, db.getDatabase());
    m_tableModel->setTable("layoutItem");
    m_tableModel->select();

    this->setSourceModel(m_tableModel);
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(onNewMessage(UDPMessage)));
    connect(TcpServer::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(onNewMessage(UDPMessage)));
    initArrays();
}

QHash<int, QByteArray> TurnoutModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("id");
    roleNames[Qt::UserRole + 1] = QByteArray("itemName");
    roleNames[Qt::UserRole + 2] = QByteArray("itemDescription");
    roleNames[Qt::UserRole + 3] = QByteArray("currentState");
//    roleNames[Qt::UserRole + 4] = QByteArray("serialNumber");
//    roleNames[Qt::UserRole + 5] = QByteArray("currentStatus");
//    roleNames[Qt::UserRole + 6] = QByteArray("version");

    return roleNames;
}


void TurnoutModel::setControllerID(int value)
{
    if(m_controllerID != value)
    {
        m_controllerID = value;
        emit controllerIDChanged();
        invalidateFilter();
    }
}

QVariant TurnoutModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    QModelIndex i;
    i = this->index(index.row(), m_tableModel->fieldIndex("id"));
    v = QSortFilterProxyModel::data(i, Qt::EditRole);
    int itemID = QSortFilterProxyModel::data(i, Qt::EditRole).toInt();
    if(role >= Qt::UserRole)
    {
        int col = role - Qt::UserRole;
        switch(col)
        {
        case 0:
            v = itemID;
            break;
        case 1:
            i = this->index(index.row(), m_tableModel->fieldIndex("itemName"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 2:
            i = this->index(index.row(), m_tableModel->fieldIndex("itemDescription"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 3:
            v = m_statusMap.value(itemID);
            break;
        default:
            break;
        }
    }
    else
    {
        v = QSortFilterProxyModel::data(index, role);
    }

    return v;
}

bool TurnoutModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

bool TurnoutModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    bool ret = true;
    if(m_controllerID > 0)
    {
        QModelIndex i = m_tableModel->index(source_row, m_tableModel->fieldIndex("controllerID"));
        if(m_tableModel->data(i, Qt::EditRole).toInt() != m_controllerID)
            ret = false;
    }
    return ret;
}

void TurnoutModel::onNewMessage(const UDPMessage &message)
{
    if(message.getMessageID() == TRN_STATUS)
    {
        if(message.getIntValue1() > 0)
            m_statusMap[message.getIntValue1()] = QString("%1").arg(message.getByteValue1());
        if(message.getIntValue2() > 0)
            m_statusMap[message.getIntValue2()] = QString("%1").arg(message.getByteValue2());

        int rows = rowCount() - 1;
        int cols = columnCount() - 1;
        QModelIndex start = this->index(0, cols);
        QModelIndex end = this->index(rows, cols);
        emit dataChanged(start, end);
    }
}

QModelIndex TurnoutModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex i;
    if(column < m_tableModel->columnCount())
        i = QSortFilterProxyModel::index(row, column, parent);
    else if(column < m_tableModel->columnCount() + 1)
        i = createIndex(row, column);

    return i;
}

int TurnoutModel::columnCount(const QModelIndex &parent) const
{
    return m_tableModel->columnCount(parent) + 1;
}

int TurnoutModel::rowCount(const QModelIndex &) const
{
    return 2;
}

QModelIndex TurnoutModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if(proxyIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapToSource(proxyIndex);
    else
        return QModelIndex(proxyIndex);
}

QModelIndex TurnoutModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if(sourceIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapFromSource(sourceIndex);
    else
        return QModelIndex(sourceIndex);
}

void TurnoutModel::initArrays()
{
    m_statusMap.clear();

    for(int x = 0; x < m_tableModel->rowCount(); x++)
    {
        QModelIndex i = m_tableModel->index(x, m_tableModel->fieldIndex("id"));
        int itemID = m_tableModel->data(i, Qt::EditRole).toInt();

        m_statusMap[itemID] = "?";
    }
}

QModelIndex TurnoutModel::parent(const QModelIndex &child) const
{
    if(child.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::parent(child);
    else
        return QModelIndex();
}


