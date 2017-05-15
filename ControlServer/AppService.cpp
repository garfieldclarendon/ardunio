#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QThreadPool>
#include <QDir>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>
#include <QSettings>

#include "AppService.h"
#include "Database.h"
#include "WebServer.h"
#include "NotificationServer.h"
#include "MessageBroadcaster.h"
#include "DeviceManager.h"
#include "ControllerManager.h"

#include "TurnoutHandler.h"
#include "ControllerHandler.h"
#include "RouteHandler.h"
#include "PanelHandler.h"
#include "SemaphoreHandler.h"

#ifdef Q_OS_WIN
#include <windows.h>

BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

#endif

CAppService::CAppService(int argc, char **argv, const QString &name, const QString &description)
    : QtService<QCoreApplication>(argc, argv, name),
      m_initialized(false)
{
    logMessage("starting");
    QString m = QString("%1 Server Starting.").arg(name);
    qDebug(m.toLatin1());
    setServiceDescription(description);
    setStartupType(QtServiceController::AutoStartup);
    QThreadPool::globalInstance()->setExpiryTimeout(-1);
}

CAppService::~CAppService(void)
{
}

void CAppService::initiateStop()
{
    QTimer::singleShot(100, this, SLOT(stopTimerProc()));
}

void CAppService::start(void)
{
    qDebug(QObject::tr("Server started. START function").toLatin1());
    WebServer::instance();
    NotificationServer::instance();
    // Force the device manager to initialize
    DeviceManager::instance();
    // Force the controller manager to initialize
    ControllerManager::instance();
    // Force the Route handler to initialize
    RouteHandler::instance();

    if(!m_initialized)
    {
#ifdef Q_OS_WIN
        SetConsoleCtrlHandler(NULL, FALSE);
        SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#endif
        QDir::setCurrent(QCoreApplication::applicationDirPath());

        Database db;
        QString path = QString("%1/Data").arg(QDir::current().absolutePath());
        QDir dir;
        dir.mkdir(path);
        db.init(path +"/RRDatabase.db");

        MessageBroadcaster::instance();

        startWebServer();

        logMessage(QObject::tr("Server started."));
        QString m = QString("Server Started.");
        qDebug(m.toLatin1());
    }
    else
    {
        QTimer::singleShot(200, this, SLOT(timerProc()));
    }
}

void CAppService::stop(void)
{	
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

void CAppService::timerProc(void)
{
    start();
}

void CAppService::stopTimerProc()
{
    stop();
}

void CAppService::startWebServer()
{
    WebServer *webServer = WebServer::instance();
    NotificationServer *notificationServer = NotificationServer::instance();

    QSettings settings("AppServer.ini", QSettings::IniFormat);
    int httpPort = settings.value("httpPort", 8080).toInt();
    int notificationPort = httpPort + 1;
    if(httpPort > 0)
    {
        UrlHandler *handler;

        ControllerHandler *controllerHandler = new ControllerHandler(this);
        handler = webServer->createUrlHandler("/controller");
        connect(handler, SIGNAL(handleUrl(NetActionType,QUrl,QString,QString&)), controllerHandler, SLOT(handleUrl(NetActionType,QUrl,QString,QString&)), Qt::DirectConnection);
        handler = webServer->createUrlHandler("/controller/config");
        connect(handler, SIGNAL(handleUrl(NetActionType,QUrl,QString,QString&)), controllerHandler, SLOT(handleConfigUrl(NetActionType,QUrl,QString,QString&)), Qt::DirectConnection);

        TurnoutHandler *turnoutHandler = new TurnoutHandler(this);
        connect(ControllerManager::instance(), &ControllerManager::newMessage, turnoutHandler, &TurnoutHandler::newMessage, Qt::QueuedConnection);
        connect(turnoutHandler, &TurnoutHandler::sendNotificationMessage, notificationServer, &NotificationServer::sendNotificationMessage);

        PanelHandler *panelHandler = new PanelHandler(this);
        connect(ControllerManager::instance(), &ControllerManager::newMessage, panelHandler, &PanelHandler::newMessage, Qt::QueuedConnection);

        SemaphoreHandler *semaphoreHandler = new SemaphoreHandler(this);
        Q_UNUSED(semaphoreHandler);

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
        CAppService *service = dynamic_cast<CAppService *>(QtServiceBase::instance());

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
