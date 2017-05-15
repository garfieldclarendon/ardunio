#pragma once
#include "qtservice.h"

class QTimer;
class WebServer;
class NotificationServer;

class CAppService : public QObject,
        public QtService<QCoreApplication>
{
    Q_OBJECT
public:
    CAppService(int argc, char **argv, const QString &name, const QString &description);
    ~CAppService(void);

    void initiateStop(void);

protected slots:
    void timerProc(void);
    void stopTimerProc(void);

protected:
    void startWebServer(void);

    // QtService overrides
    void start(void);
    void stop(void);

private:
    quint16 m_basePort;
    bool m_initialized;
};
