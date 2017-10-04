#include <QSqlTableModel>

#include "PanelOutputModel.h"

PanelOutputModel::PanelOutputModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_panelModuleID(0)
{
}


QHash<int, QByteArray> PanelOutputModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + 0] = QByteArray("id");
    roleNames[Qt::UserRole + 1] = QByteArray("panelModuleID");
    roleNames[Qt::UserRole + 2] = QByteArray("outputName");
    roleNames[Qt::UserRole + 3] = QByteArray("outputDescription");
    roleNames[Qt::UserRole + 4] = QByteArray("itemID");
    roleNames[Qt::UserRole + 5] = QByteArray("itemType");
    roleNames[Qt::UserRole + 6] = QByteArray("onValue");
    roleNames[Qt::UserRole + 7] = QByteArray("flashingValue");

    return roleNames;
}


void PanelOutputModel::setPanelModuleID(int value)
{
    if(m_panelModuleID != value)
    {
        m_panelModuleID = value;
        emit panelModuleIDChanged();
        invalidateFilter();
    }
}

QVariant PanelOutputModel::data(const QModelIndex &index, int role) const
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
            i = this->index(index.row(), m_tableModel->fieldIndex("outputName"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 3:
            i = this->index(index.row(), m_tableModel->fieldIndex("outputDescription"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 4:
            i = this->index(index.row(), m_tableModel->fieldIndex("itemID"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 5:
            i = this->index(index.row(), m_tableModel->fieldIndex("itemType"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 6:
            i = this->index(index.row(), m_tableModel->fieldIndex("onValue"));
            v = QSortFilterProxyModel::data(i, Qt::EditRole);
            break;
        case 7:
            i = this->index(index.row(), m_tableModel->fieldIndex("flashingValue"));
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

bool PanelOutputModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

bool PanelOutputModel::filterAcceptsRow(int source_row, const QModelIndex &) const
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
