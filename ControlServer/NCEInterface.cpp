#include <QSerialPortInfo>
#include <QSettings>
#include <QTimer>
#include <QApplication>

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

    connect(m_pollThread, SIGNAL(bufferInitialized()), this, SIGNAL(bufferInitialized()));
    connect(m_pollThread, SIGNAL(bufferDataChanged(quint8,int,int)), this, SIGNAL(bufferDataChanged(quint8,int,int)));

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

SerialPortThread::SerialPortThread(QObject *parent)
    : QThread(parent), m_firstTime(true), m_timerID(0)
{
    memset(m_nceBuffer, 0, NUM_BLOCK * BLOCK_LEN);
    memset(m_pollBuffer, 0, NUM_BLOCK * BLOCK_LEN);
}

SerialPortThread::~SerialPortThread()
{
    delete m_serialPort;
}

void SerialPortThread::run()
{
    openPort();
    m_timer = new QTimer(QThread::currentThread());
    m_timer->moveToThread(QThread::currentThread());
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timerProc()), Qt::QueuedConnection);
    m_timer->start(2000);

//    QTimer::singleShot(200, this, SLOT(timerProc()));
//    m_timerID = startTimer(200);
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
//    m_timer->stop();
    if(m_serialPort->isOpen())
    {
        pollRouteChanges();
    }
//    m_timer->start(2000);
//    QTimer::singleShot(200, this, SLOT(timerProc()));
}

void SerialPortThread::openPort()
{
    QSettings settings("AppServer.ini", QSettings::IniFormat);
    QString serialPort = settings.value("serialPort", "COM5").toString();

    m_serialPort = new QSerialPort;
//    m_serialPort->moveToThread(this);
    m_serialPort->setPortName(serialPort);
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::HardwareControl);
//    m_serialPort->setRequestToSend(true);
//    m_serialPort->setDataTerminalReady(true);

    if(m_serialPort->open(QIODevice::ReadWrite))
    {
        m_serialPort->setTextModeEnabled(false);
        qDebug("NCE Serail Port OPEN!");

        AddressUnion a;
        a.addressInt = CS_ACCY_MEMORY;

        NCEMessage message;
        message.accMemoryRead(CS_ACCY_MEMORY);
        int size = message.getMessageData().length();
        m_serialPort->write((const char *)message.getMessageData().constData(), 3);
        m_serialPort->waitForReadyRead(5000);
        size = m_serialPort->bytesAvailable();
        while(size < 16)
        {
            QApplication::processEvents();
            size = m_serialPort->bytesAvailable();
        }
        QByteArray versionData = m_serialPort->readAll();
        Q_UNUSED(versionData);
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
        // Copy receive data into buffer
        // Process the bytes that have changed
        QVector<quint8> data = message.getResultData();
        if(data.size() == REPLY_LEN)
        {
            for (int i = 0; i < REPLY_LEN; i++)
            {
                emit bufferDataChanged(data[i], x, i);
                if(m_firstTime == false)
                {
                    if(m_nceBuffer[i + x * BLOCK_LEN] != data[i])
                    {
                        processRouteBlock(data[i], x, i);
                    }
                }
                m_nceBuffer[i + x * BLOCK_LEN] = data[i];
            }
        }
    }

    if(m_firstTime)
        emit bufferInitialized();

    m_firstTime = false;
}

void SerialPortThread::processRouteBlock(const quint8 data, int blockIndex, int byteIndex)
{
    for(int bit = 0; bit < 8; bit++)
    {
        int routeID = 1 + bit + (byteIndex * 8) + (blockIndex * 128);
        if(((data >> bit) & 0x01) == 0)
        {
            if(RouteHandler::instance()->isRouteActive(routeID) == false)
            {
                RouteHandler::instance()->activateRoute(routeID);
            }
            // Reset the accessory entry back to normal
            NCEMessage message;
            message.accDecoder(routeID, true);
            sendMessageInternal(message);
        }
    }
}

void SerialPortThread::sendMessageInternal(NCEMessage &message)
{
    QVector<quint8> a = message.getMessageData();
    const quint8 *p = a.data();
    m_serialPort->write((const char *)p, a.size());
    m_serialPort->waitForReadyRead(5000);
    int size = m_serialPort->bytesAvailable();
    int timeout = 0;
    while(size < message.getExpectedSize() && timeout < 5000)
    {
        QApplication::processEvents();
        wait(100);
        timeout += 100;
        size = m_serialPort->bytesAvailable();
    }
    QByteArray data = m_serialPort->readAll();
    QVector<quint8> result;
    result.resize(data.size());
    for(int x = 0; x < data.size(); x++)
        result[x] = data[x];
    message.setResultData(result);
    //    size = m_serialPort->bytesAvailable();
}

void SerialPortThread::timerEvent(QTimerEvent *)
{
}
