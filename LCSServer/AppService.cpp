#include <QTranslator>
#include <QLocale>
#include <QThreadPool>
#include <QDir>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>
#include <QSettings>
#include <QProcess>

#include "AppService.h"
#include "Database.h"
#include "WebServer.h"
#include "NotificationServer.h"
#include "MessageBroadcaster.h"
#include "DeviceManager.h"
#include "ControllerManager.h"
#include "NCEInterface.h"
#include "Simulator.h"

#include "APIController.h"
#include "APIDevice.h"
#include "APITurnout.h"
#include "APIRoute.h"
#include "APISignal.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include "StatusDialog.h"

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
#endif

AppService::AppService(int argc, char **argv, const QString &name, const QString &description)
#ifdef Q_OS_WIN
    : QtService<QApplication>(argc, argv, name),
#else
    : QtService<QCoreApplication>(argc, argv, name),
#endif
      m_initialized(false), m_shutdownPi(false), m_restartPi(false), m_startSimulator(false)
{
    logMessage("starting");
    QString m = QString("%1 Starting.").arg(name);
    qDebug(m.toLatin1());
    setServiceDescription(description);
    setStartupType(QtServiceController::AutoStartup);
    QThreadPool::globalInstance()->setExpiryTimeout(-1);

    m_shutdownTimer.setInterval(30000);
    m_shutdownTimer.stop();
    connect(&m_shutdownTimer, SIGNAL(timeout()), this, SLOT(stopTimerProc()));

    m_restartTimer.setInterval(3000);
    m_restartTimer.stop();
    connect(&m_restartTimer, SIGNAL(timeout()), this, SLOT(stopTimerProc()));
}

AppService::~AppService(void)
{
}

void AppService::initiateStop()
{
    QTimer::singleShot(100, this, SLOT(stopTimerProc()));
}

void AppService::startSimulator()
{
    m_startSimulator = true;
}

void AppService::start(void)
{
    qDebug(QObject::tr("Server started. START function").toLatin1());

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
    NCEInterface::instance()->setup();

    WebServer::instance();

    NotificationServer::instance();

    // Force the device manager to initialize
    DeviceManager::instance();
    // Force the controller manager to initialize
    ControllerManager::instance();

    if(!m_initialized)
    {
#ifdef Q_OS_WIN
        SetConsoleCtrlHandler(NULL, FALSE);
        SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#endif

#ifdef Q_OS_UNIX
        m_gpio.monitorPin(23);
        m_gpio.monitorPin(24);
        connect(&m_gpio, SIGNAL(pinChanged(int,int)), this, SLOT(shutdownMonitor(int,int)));
#endif
        QDir::setCurrent(QCoreApplication::applicationDirPath());

        Database db;
        QString path = QString("%1/Data").arg(QDir::current().absolutePath());
        QDir dir;
        dir.mkdir(path);
        db.init(path +"/lcs.db");

        startWebServer();

        logMessage(QObject::tr("Server started."));
        QString m = QString("Server Started.");
        qDebug(m.toLatin1());
#ifdef Q_OS_WIN
        StatusDialog *dlg = new StatusDialog;
        dlg->show();
#endif
        if(m_startSimulator)
        {
            Simulator *simulator = new Simulator(this);
            Q_UNUSED(simulator)
        }
    }
    else
    {
        QTimer::singleShot(200, this, SLOT(timerProc()));
    }
}

void AppService::stop(void)
{	
    UDPMessage message;

    message.setMessageID(SYS_SERVER_SHUTDOWN);
    MessageBroadcaster::instance()->sendUDPMessage(message);

    QStringList l = QSqlDatabase::connectionNames();
    for(int x = 0; x < l.count(); x++)
    {
        QSqlDatabase db = QSqlDatabase::database(l.value(x));
        if(db.isValid())
        {
            qDebug(QString("Closing database: %1").arg(db.databaseName()).toLatin1());
            db.close();
        }
    }
    logMessage(QObject::tr("Server stopped"));
}

void AppService::timerProc(void)
{
    start();
}

void AppService::stopTimerProc()
{
    stop();
    if(m_shutdownPi)
    {
        QProcess process;
        process.startDetached("sudo shutdown -h now");
    }
    else if(m_restartPi)
    {
        QProcess process;
        process.startDetached("sudo shutdown -r now");
    }
}

void AppService::shutdownMonitor(int pin, int value)
{
    qDebug(QString("shutdownMonitor: pin %1 value %2").arg(pin).arg(value).toLatin1());

    if(pin == 24)
    {
        m_shutdownTimer.stop();
        if(value == 0)
        {
            m_shutdownPi = true;
            m_shutdownTimer.start();
        }
        else
        {
            m_shutdownPi = false;
        }
    }
    else if(pin == 23)
    {
        m_restartTimer.stop();
        if(value == 1)
        {
            m_restartPi = true;
            m_restartTimer.start();
        }
        else
        {
            m_restartPi = false;
        }
    }
}

void AppService::aboutToQuit()
{
    UDPMessage message;

    message.setMessageID(SYS_SERVER_SHUTDOWN);
    MessageBroadcaster::instance()->sendUDPMessage(message);
}

void AppService::startWebServer()
{
    WebServer *webServer = WebServer::instance();
    NotificationServer *notificationServer = NotificationServer::instance();

    QSettings settings("AppServer.ini", QSettings::IniFormat);
    int httpPort = settings.value("httpPort", 8080).toInt();
    int notificationPort = httpPort + 1;
    if(httpPort > 0)
    {
        APIController *controllerHandler = new APIController(this);
        Q_UNUSED(controllerHandler);

        APIDevice *deviceHandler = new APIDevice;
        Q_UNUSED(deviceHandler);

        APITurnout *turnoutHandler = new APITurnout;
        Q_UNUSED(turnoutHandler);

        APIRoute  *routeHandler = new APIRoute;
        Q_UNUSED(routeHandler);

        APISignal *signalHandler = new APISignal;
        Q_UNUSED(signalHandler);

        qDebug(QObject::tr("ready to start HTTP server").toLatin1());
        webServer->startServer(httpPort);

        qDebug(QObject::tr("ready to start Notification server").toLatin1());
        notificationServer->startServer(notificationPort);
    }
}

#ifdef Q_OS_WIN
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    qDebug("ConsoleCtrlHandler called");
    if (dwCtrlType == CTRL_C_EVENT ||
        dwCtrlType == CTRL_BREAK_EVENT ||
        dwCtrlType == CTRL_CLOSE_EVENT)
    {
        AppService *service = dynamic_cast<AppService *>(QtServiceBase::instance());

        if(service != NULL)
        {
            qDebug("Stopping service");
            service->initiateStop();
            qDebug("Calling it quits!");
            qApp->quit();
        }

        return TRUE;
    }
    return FALSE;
}
#endif
