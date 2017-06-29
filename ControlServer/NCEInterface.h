#ifndef NCEINTERFACE_H
#define NCEINTERFACE_H

#include <QObject>
#include <QSerialPort>
#include <QThread>

#include "NCEMessage.h"

class SerialPortThread : public QThread
{
    Q_OBJECT
public:
    const int CS_ACCY_MEMORY = 0xEC00;
    const int NUM_BLOCK = 16;
    const int BLOCK_LEN = 16;
    const int REPLY_LEN = 16;

    SerialPortThread(QObject *parent) : QThread(parent) { }
    ~SerialPortThread(void);
    virtual void run(void) Q_DECL_OVERRIDE;

    void setQuit(void)
    {
        quit();
        this->wait();
    }

signals:
    void newMessage(const NCEMessage &message);

public slots:
    void sendMessage(const NCEMessage &message);
    void timerProc(void);

private:
    void openPort(void);
    void pollRouteChanges(void);
    void processRouteBlock(const QByteArray &blockData, int blockIndex);
    void sendMessageInternal(NCEMessage &message);

    QSerialPort *m_serialPort;
};

class NCEInterface : public QObject
{
    Q_OBJECT
public:
    explicit NCEInterface(QObject *parent = 0);
    ~NCEInterface(void);

    void setup(void);
    void postMessage(const NCEMessage &message);
    void sendMessage(const NCEMessage &message);

    static NCEInterface *instance(void);

signals:
    void sendMessageSignal(const NCEMessage &message);

public slots:
    void newMessageSlot(const NCEMessage &message);
    void routeStatusChanged(int routeID, bool isActive);
    void deviceStatusChanged(int deviceID, int status);

private:
    static NCEInterface *m_instance;

    SerialPortThread *m_pollThread;
};

#endif // NCEINTERFACE_H
