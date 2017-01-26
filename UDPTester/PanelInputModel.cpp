#include <QSqlTableModel>

#include "PanelInputModel.h"
#include "Database.h"

PanelInputModel::PanelInputModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_panelModuleID(0)
{
    Database db;
    m_tableModel = new QSqlTableModel(this, db.getDatabase());
    m_tableModel->setTable("panelInputEntry");
    m_tableModel->setSort(m_tableModel->fieldIndex("pinIndex"), Qt::AscendingOrder);
    m_tableModel->select();

    this->setSourceModel(m_tableModel);
}

QHash<int, QByteArray> PanelInputModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("id");
    roleNames[Qt::UserRole + 1] = QByteArray("panelModuleID");
    roleNames[Qt::UserRole + 2] = QByteArray("inputName");
    roleNames[Qt::UserRole + 3] = QByteArray("inputDescription");
    roleNames[Qt::UserRole + 4] = QByteArray("inputID");
    roleNames[Qt::UserRole + 5] = QByteArray("inputType");
//    roleNames[Qt::UserRole + 6] = QByteArray("version");

    return roleNames;
}


void PanelInputModel::setPanelModuleID(int value)
{
    if(m_panelModuleID != value)
    {
        m_panelModuleID = value;
        emit panelModuleIDChanged();
        invalidateFilter();
    }
}

QVariant PanelInputModel::data(const QModelIndex &index, int role) const
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
            i = this->index(index.row(), m_tableModel->fieldIndex("panelModuleID"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 2:
            i = this->index(index.row(), m_tableModel->fieldIndex("inputName"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 3:
            i = this->index(index.row(), m_tableModel->fieldIndex("inputDescription"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 4:
            i = this->index(index.row(), m_tableModel->fieldIndex("inputID"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 5:
            i = this->index(index.row(), m_tableModel->fieldIndex("inputType"));
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

bool PanelInputModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

bool PanelInputModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    bool ret = true;
    if(m_panelModuleID > 0)
    {
        QModelIndex i = m_tableModel->index(source_row, m_tableModel->fieldIndex("panelModuleID"));
        if(m_tableModel->data(i, Qt::EditRole).toInt() != m_panelModuleID)
            ret = false;
    }
    return ret;
}
