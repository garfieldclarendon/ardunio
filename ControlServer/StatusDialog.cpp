#include <QTime>

#include "StatusDialog.h"
#include "ui_StatusDialog.h"
#include "ControllerManager.h"
#include "DeviceManager.h"
#include "PanelHandler.h"

StatusDialog::StatusDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatusDialog)
{
    ui->setupUi(this);

    setupControllerList();
    setupDeviceList();
    setupPanelList();
}

StatusDialog::~StatusDialog()
{
    delete ui;
}

void StatusDialog::onControllerConnected(int index)
{
    if(index >= ui->controllerTable->rowCount())
    {
        QString serialNumberText(QString("%1").arg(ControllerManager::instance()->getConnectionSerialNumber(index)));
        QTableWidgetItem *item;

        ui->controllerTable->insertRow(ui->controllerTable->rowCount());
        item = new QTableWidgetItem(serialNumberText);
        ui->controllerTable->setItem(ui->controllerTable->rowCount() - 1, 0, item);
        item = new QTableWidgetItem(ControllerManager::instance()->getControllerIPAddress(serialNumberText.toInt()));
        ui->controllerTable->setItem(ui->controllerTable->rowCount() - 1, 1, item);
        item = new QTableWidgetItem("00:00:00");
        ui->controllerTable->setItem(ui->controllerTable->rowCount() - 1, 2, item);
    }
}

void StatusDialog::onControllerDisconnected(int index)
{
    ui->controllerTable->removeRow(index);
}

void StatusDialog::onControllerPing(int index, quint64 length)
{
    QTableWidgetItem *item = ui->controllerTable->item(index, 2);

    if(item)
    {
        QString text;
        text = QString("%1 (%2 ms)").arg(QTime::currentTime().toString()).arg(length);
        item->setText(text);
    }
    else
    {
        onControllerConnected(index);
        item = ui->controllerTable->item(index, 2);
        if(item)
        {
            QString text;
            text = QString("%1 (%2 ms)").arg(QTime::currentTime().toString()).arg(length);
            item->setText(text);
        }
    }
}

void StatusDialog::onDeviceStatusChanged(int deviceID, int status)
{
    QString deviceIDText(QString("%1").arg(deviceID));
    QTableWidgetItem *item(NULL);

    for(int x = 0; x < ui->deviceTable->rowCount(); x++)
    {
        if(ui->deviceTable->item(x, 0)->text() == deviceIDText)
        {
            item = ui->deviceTable->item(x, 0);
            break;
        }
    }

    if(item == NULL)
    {
        ui->deviceTable->insertRow(ui->deviceTable->rowCount());
        item = new QTableWidgetItem(deviceIDText);
        ui->deviceTable->setItem(ui->deviceTable->rowCount() - 1, 0, item);
        item = new QTableWidgetItem(QString("%1").arg(status));
        ui->deviceTable->setItem(ui->deviceTable->rowCount() - 1, 1, item);
    }
    else
    {
        item = ui->deviceTable->item(item->row(), 1);
        item->setText(QString("%1").arg(status));
    }
}

void StatusDialog::onPinStateChanged(int moduleIndex, int pinNumber, int pinMode)
{
    QString moduleIndexTxt(QString("%1").arg(moduleIndex));
    QString pinNumberTxt(QString("%1").arg(pinNumber));
    QString pinModeTxt(QString("%1").arg(pinMode));

    QTableWidgetItem *item(NULL);

    for(int x = 0; x < ui->panelTable->rowCount(); x++)
    {
        if(ui->panelTable->item(x, 0)->text() == moduleIndexTxt &&
                ui->panelTable->item(x, 1)->text() == pinNumberTxt)
        {
            item = ui->deviceTable->item(x, 0);
            break;
        }
    }

    if(item == NULL)
    {
        ui->panelTable->insertRow(ui->panelTable->rowCount());
        item = new QTableWidgetItem(moduleIndexTxt);
        ui->panelTable->setItem(ui->panelTable->rowCount() - 1, 0, item);
        item = new QTableWidgetItem(pinNumberTxt);
        ui->panelTable->setItem(ui->panelTable->rowCount() - 1, 1, item);
        item = new QTableWidgetItem(pinModeTxt);
        ui->panelTable->setItem(ui->panelTable->rowCount() - 1, 2, item);
    }
    else
    {
        item = ui->panelTable->item(item->row(), 2);
        item->setText(pinModeTxt);
    }
}

void StatusDialog::setupControllerList()
{
    QStringList header;
    header << "Serial Number" << "Address" << "Ping";
    ui->controllerTable->setColumnCount(header.count());
    ui->controllerTable->setHorizontalHeaderLabels(header);
    ui->controllerTable->horizontalHeader()->setStyleSheet("color: blue");
    ui->controllerTable->verticalHeader()->setStyleSheet("color: blue");

    connect(ControllerManager::instance(), &ControllerManager::controllerConnected, this, &StatusDialog::onControllerConnected);
    connect(ControllerManager::instance(), &ControllerManager::controllerDisconnected, this, &StatusDialog::onControllerDisconnected);
    connect(ControllerManager::instance(), &ControllerManager::controllerPing, this, &StatusDialog::onControllerPing);

    for(int x = 0; x < ControllerManager::instance()->getConnectionCount(); x++)
        onControllerConnected(ControllerManager::instance()->getConnectionSerialNumber(x));
}

void StatusDialog::setupDeviceList()
{
    QStringList header;
    header << "Device ID" << "Status";
    ui->deviceTable->setColumnCount(header.count());
    ui->deviceTable->setHorizontalHeaderLabels(header);
    ui->deviceTable->horizontalHeader()->setStyleSheet("color: blue");
    ui->deviceTable->verticalHeader()->setStyleSheet("color: blue");

    connect(DeviceManager::instance(), &DeviceManager::deviceStatusChanged, this, &StatusDialog::onDeviceStatusChanged);

    for(int x = 0; x < DeviceManager::instance()->getDeviceCount(); x++)
    {
        int id = DeviceManager::instance()->getDeviceID(x);
        onDeviceStatusChanged(id, DeviceManager::instance()->getDeviceStatus(id));
    }
}

void StatusDialog::setupPanelList()
{
    QStringList header;
    header << "Module Index" << "Pin" << "Status";
    ui->panelTable->setColumnCount(header.count());
    ui->panelTable->setHorizontalHeaderLabels(header);
    ui->panelTable->horizontalHeader()->setStyleSheet("color: blue");
    ui->panelTable->verticalHeader()->setStyleSheet("color: blue");

    PanelHandler *handler = qobject_cast<PanelHandler *>(DeviceManager::instance()->getHandler(ClassPanel));
    connect(handler, &PanelHandler::pinStateChanged, this, &StatusDialog::onPinStateChanged);
}
