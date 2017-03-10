#include <QSqlTableModel>
#include <QSqlError>
#include <QTimer>
#include <QSqlQuery>

#include "ControllerModuleModel.h"
#include "Database.h"

ControllerModuleModel::ControllerModuleModel(QObject *parent)
    : QSortFilterProxyModel(parent), m_controllerID(0), m_controllerClass(-1)
{
    Database db;
    m_tableModel = new QSqlTableModel(this, db.getDatabase());
    m_tableModel->setTable("controllerModule");
    m_tableModel->select();
    if(m_tableModel->select() == false)
    {
        qDebug("ERROR LOADING controllerModule MODEL");
        qDebug(db.getDatabase().lastError().text().toLatin1());
    }

    this->setSourceModel(m_tableModel);
    qDebug(QString("controllerModule MODEL OPEN.  TOTAL ROWS: %1").arg(this->rowCount()).toLatin1());
}

QHash<int, QByteArray> ControllerModuleModel::roleNames(void) const
{
    QHash<int, QByteArray> roleNames;

    roleNames[Qt::UserRole + m_tableModel->fieldIndex("moduleClass")] = QByteArray("moduleClass");
    roleNames[Qt::UserRole + m_tableModel->fieldIndex("controllerID")] = QByteArray("controllerID");
    roleNames[Qt::UserRole + m_tableModel->fieldIndex("id")] = QByteArray("id");
    roleNames[Qt::UserRole + m_tableModel->fieldIndex("moduleIndex")] = QByteArray("moduleIndex");
    roleNames[Qt::UserRole + m_tableModel->fieldIndex("moduleName")] = QByteArray("moduleName");

    return roleNames;
}


void ControllerModuleModel::setControllerID(int value)
{
    if(m_controllerID != value)
    {
        m_controllerID = value;
        emit controllerIDChanged();
        invalidateFilter();
    }
}

void ControllerModuleModel::setControllerClass(int value)
{
    if(m_controllerClass != value)
    {
        m_controllerClass = value;
        emit controllerClassChanged();
        emit allowNewModuleChanged();
        QTimer::singleShot(1000, this, SLOT(timerProc()));
    }
}

int ControllerModuleModel::addNew(const QString &moduleName, int moduleIndex, int moduleClass)
{
    int row(-1);
    if(m_controllerID > 0)
    {
        int maxRows = 1;
        if(m_controllerClass == ClassMulti || m_controllerClass == ClassPanel)
            maxRows = MAX_MODULES;

        if(rowCount() < maxRows)
        {
            if(moduleClass <= 0)
                moduleClass = m_controllerClass;
            row = rowCount();
            insertRow(row);
            QModelIndex i;
            i = this->index(row, m_tableModel->fieldIndex("controllerID"));
            QSortFilterProxyModel::setData(i, m_controllerID);
            i = this->index(row, m_tableModel->fieldIndex("moduleClass"));
            QSortFilterProxyModel::setData(i, moduleClass);
            i = this->index(row, m_tableModel->fieldIndex("moduleIndex"));
            QSortFilterProxyModel::setData(i, moduleIndex);
            i = this->index(row, m_tableModel->fieldIndex("moduleName"));
            QSortFilterProxyModel::setData(i, moduleName);
            m_tableModel->submitAll();
        }
    }
    return row;
}

int ControllerModuleModel::createDeviceEntry(int moduleID, int moduleIndex, const QString &deviceName)
{
    Database db;
    QSqlQuery query(db.getDatabase());
    query.exec(QString("INSERT device (moduleIndex, controllerModuleID, deviceName) VALUES(%1, %2, '%3')").arg(moduleIndex).arg(moduleID).arg(deviceName));
    return 0;
}

QVariant ControllerModuleModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    QModelIndex i;
    i = this->index(index.row(), m_tableModel->fieldIndex("id"));
    v = QSortFilterProxyModel::data(i, Qt::EditRole);
    if(role >= Qt::UserRole)
    {
        int col = role - Qt::UserRole;
        i = this->index(index.row(), col);
        v = QSortFilterProxyModel::data(i, Qt::EditRole);
    }
    else
    {
        v = QSortFilterProxyModel::data(index, role);
    }

    return v;
}

bool ControllerModuleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QSortFilterProxyModel::setData(index, value, role);
}

bool ControllerModuleModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
    bool ret = true;
    if(m_controllerID > 0)
    {
        int col = m_tableModel->fieldIndex("controllerID");
        QModelIndex i = m_tableModel->index(source_row, col);
        qDebug(QString("FILTER ACCEPT: %1 = %2").arg(m_controllerID).arg(m_tableModel->data(i, Qt::EditRole).toInt()).toLatin1());
        if(m_tableModel->data(i, Qt::EditRole).toInt() != m_controllerID)
            ret = false;
    }
    return ret;
}

QModelIndex ControllerModuleModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex i;
    if(column < m_tableModel->columnCount())
        i = QSortFilterProxyModel::index(row, column, parent);
    else if(column < m_tableModel->columnCount() + 1)
        i = createIndex(row, column);

    return i;
}

int ControllerModuleModel::columnCount(const QModelIndex &parent) const
{
    return m_tableModel->columnCount(parent);
}

int ControllerModuleModel::rowCount(const QModelIndex &) const
{
    return QSortFilterProxyModel::rowCount();
//    return MAX_MODULES;
}

QModelIndex ControllerModuleModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if(proxyIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapToSource(proxyIndex);
    else
        return QModelIndex(proxyIndex);
}

QModelIndex ControllerModuleModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if(sourceIndex.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::mapFromSource(sourceIndex);
    else
        return QModelIndex(sourceIndex);
}


QModelIndex ControllerModuleModel::parent(const QModelIndex &child) const
{
    if(child.column() < m_tableModel->columnCount())
        return QSortFilterProxyModel::parent(child);
    else
        return QModelIndex();
}

bool ControllerModuleModel::getAllowNewModule() const
{
    bool ret = false;
    if(m_controllerClass == ClassMulti || m_controllerClass == ClassPanel)
    {
        if(rowCount() < MAX_MODULES)
            ret = true;
    }
    else
    {
        int rows = rowCount();
        if(rows == 0)
            ret = true;
    }
    return ret;
}

void ControllerModuleModel::setAllowNewModule(bool /* value */)
{
    // do nothing for now
}

int ControllerModuleModel::getModuleClass(int row) const
{
    QModelIndex i = this->index(row, m_tableModel->fieldIndex("moduleClass"));
    QVariant v = QSortFilterProxyModel::data(i, Qt::EditRole);
    return v.toInt();
}

void ControllerModuleModel::timerProc()
{
    qDebug("timerProc");
    emit allowNewModuleChanged();
}


