#include <QTableView>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QVBoxLayout>

#include "ControllerModuleWidget.h"

ControllerModuleWidget::ControllerModuleWidget(QWidget *parent) : QWidget(parent), m_controllerID(0)
{
    setupModel();
    setupUI();
}

void ControllerModuleWidget::selectionChanged()
{
    QModelIndex i = tableModel->index(tableView->currentIndex().row(), 2);
    int controllerModuleID = tableModel->data(i, Qt::EditRole).toInt();
    qDebug(QString("Controller Module %1 selected").arg(controllerModuleID).toLatin1());
    emit currentControllerModuleIDChanged(controllerModuleID, m_controllerID);
}

void ControllerModuleWidget::setCurrentControllerID(int controllerID)
{
    m_controllerID = controllerID;
    QString filter(QString("controllerID = %1").arg(controllerID));
    tableModel->setFilter(filter);
    tableView->selectRow(0);
    selectionChanged();
}

void ControllerModuleWidget::setupModel()
{
    tableModel = new QSqlRelationalTableModel(this, db.getDatabase());
    tableModel->setTable("controllerModule");
    tableModel->setRelation(0, QSqlRelation("classCode", "id", "className"));

    tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Class"));
    tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Controller ID"));
    tableModel->setHeaderData(2, Qt::Horizontal, QObject::tr("ID"));
    tableModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Index"));
    tableModel->setHeaderData(4, Qt::Horizontal, QObject::tr("Name"));

    tableModel->setFilter("controllerID = 0");
    tableModel->select();
}

void ControllerModuleWidget::setupUI()
{
    tableView = new QTableView;
    tableView->setModel(tableModel);
    tableView->setItemDelegate(new QSqlRelationalDelegate(tableView));

    connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectionChanged()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tableView);
    this->setLayout(layout);
}
