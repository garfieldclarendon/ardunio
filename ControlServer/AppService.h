#pragma once
#include "qtservice.h"
#ifdef Q_OS_WIN
#include <QApplication>
#else
#include <QCoreApplication>
#endif

#ifdef Q_OS_UNIX
#include "PIGPIO.h"
#endif

class QTimer;
class WebServer;
class NotificationServer;

class CAppService : public QObject,
#ifdef Q_OS_WIN
        public QtService<QApplication>
#else
        public QtService<QCoreApplication>
#endif
{
    Q_OBJECT
public:
    CAppService(int argc, char **argv, const QString &name, const QString &description);
    ~CAppService(void);

    void initiateStop(void);

protected slots:
    void timerProc(void);
    void stopTimerProc(void);
    void shutdownMonitor(int pin, int value);

protected:
    void startWebServer(void);

    // QtService overrides
    void start(void);
    void stop(void);

private:
    quint16 m_basePort;
    bool m_initialized;
    bool m_shutdownPi;
#ifdef Q_OS_UNIX
    PIGPIO m_gpio;
#endif
};
