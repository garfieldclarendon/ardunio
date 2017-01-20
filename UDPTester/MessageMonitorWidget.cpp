#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QListView>
#include <QStringListModel>

#include "MessageMonitorWidget.h"
#include "MessageBroadcaster.h"
#include "SystemMessageHandler.h"
#include "GlobalDefs.h"
#include "TcpServer.h"

MessageMonitorWidget::MessageMonitorWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();

    connect(MessageBroadcaster::instance(), SIGNAL(newRawUDPMessage(QString)), this, SLOT(newRawMessage(QString)));

    SystemMessageHandler *sysHandler = new SystemMessageHandler(this);
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), sysHandler, SLOT(handleMessage(UDPMessage)));

    TcpServer *tcpServer = new TcpServer(83, this);
    connect(tcpServer, SIGNAL(newMessage(UDPMessage)), sysHandler, SLOT(handleMessage(UDPMessage)));
    connect(tcpServer, SIGNAL(newRawUDPMessage(QString)), this, SLOT(newRawMessage(QString)));
}

void MessageMonitorWidget::setupUI()
{
    m_statusLabel = new QLabel("Idle", this);
    m_statusLabel->setAlignment(Qt::AlignHCenter);
    m_model = new QStringListModel(this);
    m_messages = new QListView(this);
    m_messages->setModel(m_model);

    QHBoxLayout *mainLayout = new QHBoxLayout;

    QVBoxLayout *l = new QVBoxLayout;
    QLabel *label = new QLabel("Status:");
    label->setAlignment(Qt::AlignHCenter);
    l->addWidget(label);
    l->addWidget(m_statusLabel);
    label = new QLabel("Incoming Messages:");
    label->setAlignment(Qt::AlignHCenter);
    l->addWidget(label);
    l->addWidget(m_messages, 2);

    QPushButton *clearButton = new QPushButton("Clear Messages", this);
    connect(clearButton, SIGNAL(clicked(bool)), this, SLOT(clearModel()));
    QHBoxLayout *h = new QHBoxLayout;
    h->addStretch();
    h->addWidget(clearButton);
    h->addStretch();
    l->addLayout(h);
    l->addStretch();

    mainLayout->addLayout(l, 2);

    this->setLayout(mainLayout);
}

void MessageMonitorWidget::newRawMessage(const QString &message)
{
    int row = m_model->rowCount();
    m_model->insertRow(row);
    QModelIndex index = m_model->index(row, 0);
    m_model->setData(index, message);
}

void MessageMonitorWidget::clearModel()
{
    m_model->setStringList(QStringList());
}

void MessageMonitorWidget::logError(int category, int code, const QString &errorText)
{
    int row = m_model->rowCount();
    m_model->insertRow(row);
    QModelIndex index = m_model->index(row, 0);
    m_model->setData(index, QString("DB ERROR  Category: %1, Code %2, Text: %3").arg(category).arg(code).arg(errorText));
}
