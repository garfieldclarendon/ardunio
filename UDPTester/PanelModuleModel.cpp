#include <QSqlTableModel>

#include "PanelModuleModel.h"
#include "Database.h"

PanelModuleModel::PanelModuleModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_controllerID(0)
{
    Database db;
    m_tableModel = new QSqlTableModel(this, db.getDatabase());
    m_tableModel->setTable("controllerModule");
    m_tableModel->select();

    this->setSourceModel(m_tableModel);
}

QHash<int, QByteArray> PanelModuleModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("id");
    roleNames[Qt::UserRole + 1] = QByteArray("controllerID");
    roleNames[Qt::UserRole + 2] = QByteArray("moduleIndex");
    roleNames[Qt::UserRole + 3] = QByteArray("moduleName");

    return roleNames;
}


void PanelModuleModel::setControllerID(int value)
{
    if(m_controllerID != value)
    {
        m_controllerID = value;
        invalidateFilter();
        emit controllerIDChanged();
    }
}

QVariant PanelModuleModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    QModelIndex i;
    i = this->index(index.row(), m_tableModel->fieldIndex("id"));
    v = QSortFilterProxyModel::data(i, Qt::EditRole);
    int id = QSortFilterProxyModel::data(i, Qt::EditRole).toInt();
    if(role >= Qt::UserRole)
    {
        int col = role - Qt::UserRole;
        switch(col)
        {
        case 0:
            v = id;
            break;
        case 1:
            i = this->index(index.row(), m_tableModel->fieldIndex("controllerID"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 2:
            i = this->index(index.row(), m_tableModel->fieldIndex("moduleIndex"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 3:
            i = this->index(index.row(), m_tableModel->fieldIndex("modulelName"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
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

bool PanelModuleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

QModelIndex PanelModuleModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex i;
    if(column < m_tableModel->columnCount())
        i = QSortFilterProxyModel::index(row, column, parent);
    else if(column < m_tableModel->columnCount() + 1)
        i = createIndex(row, column);

    return i;
}

int PanelModuleModel::rowCount(const QModelIndex &) const
{
    return 8;
}

QModelIndex PanelModuleModel::parent(const QModelIndex &child) const
{
    if(child.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::parent(child);
    else
        return QModelIndex();
}

bool PanelModuleModel::filterAcceptsRow(int source_row, const QModelIndex &) const
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
