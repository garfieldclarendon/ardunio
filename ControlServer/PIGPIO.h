#ifndef PIGPIO_H
#define PIGPIO_H

#include <QObject>

#define PIN_IN  0
#define PIN_OUT 1

#define PIN_LOW  0
#define PIN_HIGH 1

#define MAX_PINS 41
#define MAX_MONITOR 5

class PIGPIO : public QObject
{
    Q_OBJECT
public:
    explicit PIGPIO(QObject *parent = 0);
    ~PIGPIO(void);

    bool pinDirection(int pin, int dir);
    int readPin(int pin);
    int pinWrite(int pin, int value);

signals:
    void pinChanged(int pin, int value);

public slots:
    void monitorPin(int pin);

protected slots:
    void timerProc(void);

private:
    bool exportPin(int pin);
    bool unExportPin(int pin);
    int m_pinList[MAX_PINS];
    int m_monitorList[MAX_MONITOR];
    int m_monitorValue[MAX_MONITOR];
};

#endif // PIGPIO_H
