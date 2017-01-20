#include <QTableView>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

#include "DeviceTab.h"

DeviceTab::DeviceTab(QWidget *parent) : QWidget(parent)
{
    setupModel();
    setupUI();
}

void DeviceTab::addButtonClicked()
{
    Database db;
    int deviceID = db.getNextID("device");

    if(deviceID > 0)
    {
        int row = tableModel->rowCount();
        tableModel->insertRow(row);
        QSqlRecord r = tableModel->record(row);
        r.setValue(0, deviceID);
        tableModel->setRecord(row, r);
    }
}

void DeviceTab::deleteButtonClicked()
{

}

void DeviceTab::saveModel()
{
    bool ret = tableModel->submitAll();
    if(ret == false)
        qDebug(tableModel->lastError().text().toLatin1());
}

void DeviceTab::setupModel()
{
    tableModel = new QSqlRelationalTableModel(this, db.getDatabase());
    tableModel->setTable("device");
    tableModel->setRelation(1, QSqlRelation("controller", "id", "controllerName"));

    tableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    tableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Controller"));
    tableModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Name"));
    tableModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Description"));

    tableModel->select();
}

void DeviceTab::sendConfigClicked()
{
    int controllerID = 0;

    emit sendConfig(controllerID);
}

void DeviceTab::resetClicked()
{
    int controllerID = 0;

    emit resetController(controllerID);
}

void DeviceTab::sendFirmware()
{
    int controllerID = 0;

    emit sendFirmware(controllerID);
}

void DeviceTab::setupUI()
{
    tableView = new QTableView;
    tableView->setModel(tableModel);
    tableView->setItemDelegate(new QSqlRelationalDelegate(tableView));

    addButton = new QPushButton("Add", this);
    deleteButton = new QPushButton("Delete", this);
    deleteButton->setEnabled(false);
    saveButton = new QPushButton("Save", this);
    QPushButton *sendConfig = new QPushButton("Send Config", this);
    QPushButton *sendReset = new QPushButton("Send Reset", this);
    QPushButton *sendFirmwareUpdate = new QPushButton("Send Firmware", this);

    connect(addButton, SIGNAL(clicked(bool)), this, SLOT(addButtonClicked()));
    connect(deleteButton, SIGNAL(clicked(bool)), this, SLOT(deleteButtonClicked()));
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(saveModel()));
    connect(sendConfig, SIGNAL(clicked(bool)), this, SLOT(sendConfigClicked()));
    connect(sendReset, SIGNAL(clicked(bool)), this, SLOT(resetClicked()));
    connect(sendFirmwareUpdate, SIGNAL(clicked(bool)), this, SLOT(sendFirmware()));

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(sendConfig);
    buttonLayout->addWidget(sendReset);
    buttonLayout->addWidget(sendFirmwareUpdate);
    buttonLayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(tableView);
    layout->addLayout(buttonLayout);
    this->setLayout(layout);
}