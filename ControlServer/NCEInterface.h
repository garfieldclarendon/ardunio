#ifndef NCEINTERFACE_H
#define NCEINTERFACE_H

#include <QObject>
#include <QSerialPort>

class NCEInterface : public QObject
{
    Q_OBJECT
public:
    explicit NCEInterface(QObject *parent = 0);
    ~NCEInterface(void);

    void setup(int port);
    static NCEInterface *instance(void);

signals:

public slots:
    void readData(void);

private:
    static NCEInterface *m_instance;
    QSerialPort m_serialPort;
};

#endif // NCEINTERFACE_H
