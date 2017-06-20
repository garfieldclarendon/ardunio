#include <QSerialPortInfo>

#include "NCEInterface.h"

NCEInterface * NCEInterface::m_instance = NULL;

NCEInterface::NCEInterface(QObject *parent) : QObject(parent)
{

}

NCEInterface::~NCEInterface()
{

}

void NCEInterface::setup(int port)
{
    QSerialPortInfo info;
    QList<QSerialPortInfo> ports = info.availablePorts();
    QString name = ports.value(0).portName();
    QString name2 = ports.value(1).portName();

    m_serialPort.setPortName(name2);
    m_serialPort.setBaudRate(QSerialPort::Baud9600);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setStopBits(QSerialPort::OneStop);

    connect(&m_serialPort, &QSerialPort::readyRead, this, &NCEInterface::readData);

    if(m_serialPort.open(QIODevice::ReadWrite))
    {
        qDebug("NCE Serail Port OPEN!");

        unsigned char command[2];
        command[0] = 0xAA;
        command[1] = 2;
        m_serialPort.write((const char *)command, 1);
    }
    else
    {
        qDebug("NCE Serail Port FAILED TO OPEN!");
    }
}

NCEInterface *NCEInterface::instance()
{
    if(m_instance == NULL)
    {
        m_instance = new NCEInterface;
    }
    return m_instance;
}

void NCEInterface::readData()
{
    QByteArray data = m_serialPort.readAll();

}

