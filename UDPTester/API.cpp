#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QEventLoop>

#include "../ControlServer/MessageBroadcaster.h"

#include "API.h"

API *API::m_instance = NULL;

API::API(const QString &server, const int port, QObject *parent)
    : QObject(parent), m_server(server), m_port(port), m_notificationSocket(NULL)
{
    if(m_server.endsWith('/') || m_server.endsWith('\\'))
        m_server.chop(1);
    setupUDPSocket();
    if(m_server.length() > 0)
        setupNotificationSocket();
}

API::API(QObject *parent)
    : QObject(parent), m_notificationSocket(NULL)
{
    QSettings settings("AppServer.ini", QSettings::IniFormat);
    m_port = settings.value("httpPort", 8080).toInt();
    setupUDPSocket();
}

API::~API()
{
    if(m_notificationSocket)
        m_notificationSocket->deleteLater();
}

API *API::instance()
{
    if(m_instance == NULL)
        m_instance = new API;

    return m_instance;
}

QString API::getControllerList()
{
    QString json;
    QUrl url(buildUrl("controller_list"));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getDeviceList(ClassEnum deviceType)
{
    QString json;
    QString s("device_list");
    if(deviceType != ClassUnknown)
        s.append(QString("?classCode=%1").arg(deviceType));
    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

void API::textMessageReceived(const QString &message)
{
    QJsonDocument doc(QJsonDocument::fromJson(message.toLatin1()));
    QJsonObject obj = doc.object();
    QString urlText = obj["uri"].toString();

    if(urlText == "/api/notification/controller")
    {
        QString s = obj["serialNumber"].toString();
        int serialNumber = s.toInt();
        s = obj["status"].toString();
        ControllerStatus status = (ControllerStatus)s.toInt();
        emit controllerChanged(serialNumber, status);
    }
    else if(urlText == "/api/notification/turnout")
    {
        QString s = obj["turnoutID"].toString();
        int deviceID = s.toInt();
        s = obj["state"].toString();
        int status = s.toInt();
        emit deviceChanged(deviceID, status);
    }
}

void API::notificationStateChanged(QAbstractSocket::SocketState state)
{
    /*
    UnconnectedState,
    HostLookupState,
    ConnectingState,
    ConnectedState,
    BoundState,
    ListeningState,
    ClosingState
    */
    switch (state)
    {
    case QAbstractSocket::UnconnectedState:
        QTimer::singleShot(5000, this, SLOT(connectNotificationSocket()));
        break;
    default:
        break;
    }
}

void API::connectNotificationSocket()
{
    QUrl url(buildNotifcationUrl("notification"));
    m_notificationSocket->open(url);
}

void API::newUDPMessage(const UDPMessage &message)
{
    if(message.getMessageID() == SYS_SERVER_HEARTBEAT)
    {
        QString address = QString("%1.%2.%3.%4").arg(message.getField(0)).arg(message.getField(1)).arg(message.getField(2)).arg(message.getField(3));
        if(address != m_server)
        {
            m_server = address;
            setupNotificationSocket();
            emit apiReady();
        }
    }
}

void API::setupNotificationSocket()
{
    if(m_server.length() > 0 && m_port > 0)
    {
        if(m_notificationSocket)
            m_notificationSocket->deleteLater();
        m_notificationSocket = new QWebSocket;
        connect(m_notificationSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageReceived(QString)));
        connect(m_notificationSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(notificationStateChanged(QAbstractSocket::SocketState)));

        connectNotificationSocket();
    }
}

void API::setupUDPSocket()
{
    MessageBroadcaster::setRunAsClient(true);
    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(newUDPMessage(UDPMessage)));
    UDPMessage message;
    message.setMessageID(SYS_FIND_SERVER);
    MessageBroadcaster::instance()->sendUDPMessage(message);
}

QUrl API::buildNotifcationUrl(const QString &path)
{
    return QString("ws://%1:%2/api/%3").arg(m_server).arg(m_port + 1).arg(path);
}

QUrl API::buildUrl(const QString &path)
{
    return QString("http://%1:%2/api/%3").arg(m_server).arg(m_port).arg(path);
}

QString API::sendToServer(const QUrl &url, const QString &json, NetActionType netAction)
{
    QString ret;
    QNetworkRequest request;
    request.setUrl(url);

    QNetworkAccessManager manager;
    QNetworkReply *reply;
    if(netAction == NetActionGet)
        reply = manager.get(request);
    else if(netAction == NetActionUpdate)
        reply = manager.post(request, json.toLatin1());
    else if(netAction == NetActionAdd)
        reply = manager.put(request, json.toLatin1());

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    ret = reply->readAll();

    return ret;
}
