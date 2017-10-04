#include <QTableView>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QVBoxLayout>

#include "ControllerTab.h"

ControllerTab::ControllerTab(QWidget *parent) : QWidget(parent)
{
    setupModel();
    setupUI();
}

void ControllerTab::selectionChanged()
{
    QModelIndex i = tableModel->index(tableView->currentIndex().row(), 1);
    int controllerID = tableModel->data(i, Qt::EditRole).toInt();
    qDebug(QString("Controller %1 selected").arg(controllerID).toLatin1());
    emit currentControllerIDChanged(controllerID);
}

void ControllerTab::setupModel()
{
}

void ControllerTab::setupUI()
{
    tableView = new QTableView;
    tableView->setModel(tableModel);
    tableView->setItemDelegate(new QSqlRelationalDelegate(tableView));

    connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(selectionChanged()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tableView);
    this->setLayout(layout);
}

