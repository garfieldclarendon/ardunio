#include <QSerialPortInfo>
#include <QSettings>
#include <QTimer>

#include "NCEInterface.h"
#include "RouteHandler.h"

NCEInterface * NCEInterface::m_instance = NULL;

NCEInterface::NCEInterface(QObject *parent) : QObject(parent), m_pollThread(NULL)
{

}

NCEInterface::~NCEInterface()
{
    m_pollThread->setQuit();
}

void NCEInterface::setup(void)
{
    m_pollThread = new SerialPortThread(this);

//    connect(m_pollThread, SIGNAL(newMessage(NCEMessage)), this, SLOT(newMessageSlot(NCEMessage)), Qt::QueuedConnection);
//    connect(this, SIGNAL(sendMessageSignal(NCEMessage)), m_pollThread, SLOT(sendMessage(NCEMessage)), Qt::QueuedConnection);

    m_pollThread->start();
}

NCEInterface *NCEInterface::instance()
{
    if(m_instance == NULL)
    {
        m_instance = new NCEInterface;
    }
    return m_instance;
}

void NCEInterface::newMessageSlot(const NCEMessage & /*message*/)
{
}

void NCEInterface::routeStatusChanged(int routeID, bool isActive)
{
    NCEMessage message;
    message.accDecoder(routeID, isActive == false);

    postMessage(message);
}

void NCEInterface::deviceStatusChanged(int deviceID, int status)
{
    NCEMessage message;
    message.accDecoder(deviceID, status);

    postMessage(message);
}

void NCEInterface::postMessage(const NCEMessage &message)
{
    emit sendMessageSignal(message);
}

void NCEInterface::sendMessage(const NCEMessage &message)
{
    emit sendMessageSignal(message);
    //TODO:  Add wait code here
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SerialPortThread
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SerialPortThread::~SerialPortThread()
{
    delete m_serialPort;
}

void SerialPortThread::run()
{
    openPort();

    QTimer::singleShot(200, this, SLOT(timerProc()));
    QThread::run();
}

void SerialPortThread::sendMessage(const NCEMessage &message)
{
    NCEMessage returnMessage(message);
    if(m_serialPort->isOpen())
    {
        sendMessageInternal(returnMessage);
    }

    emit newMessage(returnMessage);
}

void SerialPortThread::timerProc()
{
    if(m_serialPort->isOpen())
    {
        pollRouteChanges();
    }

    QTimer::singleShot(200, this, SLOT(timerProc()));
}

void SerialPortThread::openPort()
{
    QSettings settings("AppServer.ini", QSettings::IniFormat);
    QString serialPort = settings.value("serialPort", "COM4").toString();

    m_serialPort = new QSerialPort;
    m_serialPort->setPortName(serialPort);
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        qDebug("NCE Serail Port OPEN!");

//        unsigned char command[3];
//        command[0] = 0x8F;
//        command[1] = CS_ACCY_MEMORY;
//        command[2] = 0;
        NCEMessage message;
        message.accMemoryRead(CS_ACCY_MEMORY);
        int size = message.getMessageData().length();
        m_serialPort->write(message.getMessageData(), size);
        m_serialPort->waitForReadyRead(5000);
        size = m_serialPort->bytesAvailable();
        while(size < 16)
            size = m_serialPort->bytesAvailable();
        QByteArray versionData = m_serialPort->readAll();
//        Q_UNUSED(versionData);
    }
    else
    {
        qDebug("NCE Serail Port FAILED TO OPEN!");
    }
}

void SerialPortThread::pollRouteChanges()
{
    for(int x = 0; x < NUM_BLOCK; x++)
    {
        int nceAccAddress = CS_ACCY_MEMORY + (x * BLOCK_LEN);
        NCEMessage message;
        message.accMemoryRead(nceAccAddress);

        sendMessageInternal(message);
        processRouteBlock(message.getResultData(), x);
    }
}

void SerialPortThread::processRouteBlock(const QByteArray &blockData, int blockIndex)
{
    for(int x = 0; x < blockData.count(); x++)
    {
        unsigned char recMemByte = blockData[x];
        for(int bit = 0; bit < 8; bit++)
        {
            int routeID = 1 + bit + (x * 8) + (blockIndex * 128);
            if(((recMemByte >> bit) & 0x01) == 1)
            {
                if(RouteHandler::instance()->isRouteActive(routeID) == false)
                {
                    RouteHandler::instance()->activateRoute(routeID);
                    // Reset the accessory entry back to normal
                    NCEMessage message;
                    message.accDecoder(routeID, true);
                    sendMessageInternal(message);
                }
            }
        }
    }
}

void SerialPortThread::sendMessageInternal(NCEMessage &message)
{
    QByteArray a = message.getMessageData();
    m_serialPort->write(a);
    m_serialPort->waitForReadyRead(5000);
    int size = m_serialPort->bytesAvailable();
    while(size < message.getExpectedSize())
        size = m_serialPort->bytesAvailable();
    message.setResultData(m_serialPort->readAll());
    size = m_serialPort->bytesAvailable();
}
