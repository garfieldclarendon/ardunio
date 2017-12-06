#pragma once

#include <QTimer>
#include "qtservice.h"

#ifdef Q_OS_WIN
#include <QApplication>
#else
#include <QCoreApplication>
#endif

#ifdef Q_OS_UNIX
#include "PIGPIO.h"
#endif

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
    void aboutToQuit(void);

protected:
    void startWebServer(void);

    // QtService overrides
    void start(void);
    void stop(void);

private:
    quint16 m_basePort;
    bool m_initialized;
    bool m_shutdownPi;
    bool m_restartPi;
    QTimer m_shutdownTimer;
    QTimer m_restartTimer;
#ifdef Q_OS_UNIX
    PIGPIO m_gpio;
#endif
};
