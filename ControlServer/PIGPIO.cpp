#include "PIGPIO.h"
#include "sys/stat.h"
#include "sys/types.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <QTimer>

PIGPIO::PIGPIO(QObject *parent) : QObject(parent)
{
    memset(m_pinList, 0, sizeof(int)*MAX_PINS);
    memset(m_monitorList, 0, sizeof(int)*MAX_MONITOR);
    memset(m_monitorValue, 0, sizeof(int)*MAX_MONITOR);
}

PIGPIO::~PIGPIO()
{
    for(int x = 0; x < MAX_PINS; x++)
    {
        if(m_pinList[x] == 1)
        {
            unExportPin(x);
        }
    }
}

bool PIGPIO::pinDirection(int pin, int dir)
{
    static const char s_directions_str[]  = "in\0out";

    if(exportPin(pin) == false)
        return false;

    m_pinList[pin] = 1;

#define DIRECTION_MAX 35
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return(-1);
    }

    if (-1 == write(fd, &s_directions_str[PIN_IN == dir ? 0 : 3], PIN_IN == dir ? 2 : 3)) {
        fprintf(stderr, "Failed to set direction!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

int PIGPIO::readPin(int pin)
{
#define VALUE_MAX 30
    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        return(-1);
    }

    if (-1 == read(fd, value_str, 3))
    {
        fprintf(stderr, "Failed to read value!\n");
        return(-1);
    }

    close(fd);

    return(atoi(value_str));
}

int PIGPIO::pinWrite(int pin, int value)
{
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return(-1);
    }

    if (1 != write(fd, &s_values_str[PIN_LOW == value ? 0 : 1], 1))
    {
        fprintf(stderr, "Failed to write value!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

void PIGPIO::monitorPin(int pin)
{
    for(int x = 0; x < MAX_MONITOR; x++)
    {
        if(m_monitorList[x] == 0)
        {
            m_monitorList[x] = pin;
            break;
        }
    }

    pinDirection(pin, PIN_IN);
    QTimer::singleShot(1000, this, SLOT(timerProc()));
}

void PIGPIO::timerProc()
{
    for(int x = 0; x < MAX_MONITOR; x++)
    {
        if(m_monitorList[x] > 0)
        {
            int pin = m_monitorList[x];
            int value = readPin(pin);
            if(value != m_monitorValue[x])
            {
                m_monitorValue[x] = value;
                emit pinChanged(pin, value);
            }
        }
    }

    QTimer::singleShot(1000, this, SLOT(timerProc()));
}

bool PIGPIO::exportPin(int pin)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open export for writing!\n");
        return(false);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(true);
}

bool PIGPIO::unExportPin(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return(false);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(true);
}

