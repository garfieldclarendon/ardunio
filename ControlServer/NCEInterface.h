#ifndef NCEINTERFACE_H
#define NCEINTERFACE_H

#include <QObject>
#include <QSerialPort>
#include <QThread>

#include "NCEMessage.h"

const unsigned int CS_ACCY_MEMORY = 0xEC00;
const int NUM_BLOCK = 1; //16;
const int BLOCK_LEN = 16;
const int REPLY_LEN = 16;

struct AddressStruct {
    quint8 byteL;
    quint8 byteH;
};

union AddressUnion {
    unsigned int addressInt;
    AddressStruct addressStruct;
};

class QTimer;

class SerialPortThread : public QThread
{
    Q_OBJECT
public:

    enum PortStatusEnum {
        Disconnected,
        Connected,
        Running
    };

    SerialPortThread(QObject *parent);
    ~SerialPortThread(void);
    virtual void run(void) Q_DECL_OVERRIDE;

    void setQuit(void)
    {
        m_quit = true;
        wait();
    }
    void openPort(void);

signals:
    void newMessage(const NCEMessage &message);
    void bufferInitialized(void);
    void bufferDataChanged(quint8 byte, int blockIndex, int byteIndex);

public slots:
    void sendMessage(const NCEMessage &message);
    void pollCommandStation(void);

private:
    void pollRouteChanges(void);
    void processRouteBlock(const quint8 data, int blockIndex, int byteIndex);
    void sendMessageInternal(NCEMessage &message);
    void checkVersionNumber(void);

    quint8 m_nceBuffer[NUM_BLOCK * BLOCK_LEN]; // Copy of NCE CS accessory memory
    quint8 m_pollBuffer[NUM_BLOCK * BLOCK_LEN]; // place to store reply messages
    bool m_firstTime;
    bool m_quit;
    QSerialPort *m_serialPort;
    PortStatusEnum m_portStatus;
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
    void bufferInitialized(void);
    void bufferDataChanged(quint8 byte, int blockIndex, int byteIndex);

public slots:
    void newMessageSlot(const NCEMessage &message);
    void routeStatusChanged(int routeID, bool isActive);
    void deviceStatusChanged(int deviceID, int status);

private:
    static NCEInterface *m_instance;

    SerialPortThread *m_pollThread;
};

#endif // NCEINTERFACE_H
