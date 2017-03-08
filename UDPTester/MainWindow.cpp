#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QDir>
#include <QStandardPaths>

#include "MainWindow.h"
#include "MessageMonitorWidget.h"
#include "TrackSchematic.h"
#include "TrackTrunout.h"
#include "ControllerTab.h"
#include "DeviceTab.h"
#include "MessageBroadcaster.h"
#include "ControllerWidget.h"
#include "GlobalDefs.h"
#include "UI.h"
#include "ControllerModuleWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    db.init(QString("%1/RRDatabase.db").arg(QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).value(0)));
    setupUI();
    MessageBroadcaster::instance(true)->enableHeartbeat(false);
}

MainWindow::~MainWindow()
{

}

void MainWindow::sendConfigData(int controllerID)
{
    UDPMessage message;
    message.setMessageID(SYS_CONFIG_CHANGED);
    message.setControllerID(controllerID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MainWindow::sendResetCommand(int controllerID)
{
    UDPMessage message;
    message.setMessageID(SYS_REBOOT_CONTROLLER);
    message.setControllerID(controllerID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MainWindow::sendFirmware(int controllerID)
{
    UDPMessage message;
    message.setMessageID(SYS_DOWNLOAD_FIRMWARE);
    message.setControllerID(controllerID);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void MainWindow::setupUI()
{
    setWindowTitle("Message Tester");

    UI ui;
    resize(ui.applyRatio(1100), ui.applyRatio(768));

    QTabWidget *tab = new QTabWidget(this);

    ControllerWidget *controllerWidget = new ControllerWidget;
    controllerWidget->setFocusPolicy(Qt::StrongFocus);
    tab->addTab(controllerWidget, "Manage Controllers");

    MessageMonitorWidget *widget = new MessageMonitorWidget(this);
    connect(&db, SIGNAL(logError(int,int,QString)), widget, SLOT(logError(int,int,QString)));
    tab->addTab(widget, "Messages");

    TrackSchematic *widget2 = new TrackSchematic(this);

    TrackTrunout *turnOut = new TrackTrunout(widget2);
    widget2->addTrackSection(turnOut);

    db.checkDatabase();

    tab->addTab(widget2, "Track Plan");

    createControllerTab(tab);

    this->setCentralWidget(tab);
}

void MainWindow::createControllerTab(QTabWidget *tab)
{
    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *rightLayout = new QVBoxLayout;

    QWidget *widget = new QWidget(this);

    ControllerTab *controllerTab = new ControllerTab(this);
    layout->addWidget(controllerTab);

    ControllerModuleWidget *moduleWidget = new ControllerModuleWidget(this);
    connect(controllerTab, SIGNAL(currentControllerIDChanged(int)), moduleWidget, SLOT(setCurrentControllerID(int)));
    rightLayout->addWidget(moduleWidget);

    DeviceTab *deviceTab = new DeviceTab(this);
    connect(moduleWidget, SIGNAL(currentControllerModuleIDChanged(int,int)), deviceTab, SLOT(setModuleID(int,int)));
    connect(deviceTab, SIGNAL(sendConfig(int)), this, SLOT(sendConfigData(int)));
    connect(deviceTab, SIGNAL(resetController(int)), this, SLOT(sendResetCommand(int)));
    connect(deviceTab, SIGNAL(sendFirmware(int)), this, SLOT(sendFirmware(int)));
    rightLayout->addWidget(deviceTab);

    layout->addLayout(rightLayout);
    widget->setLayout(layout);

    tab->addTab(widget, "Controllers");
}

void MainWindow::closeEvent(QCloseEvent *)
{
}
