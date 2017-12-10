#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QEventLoop>
#include <QJsonValue>
#include <QCoreApplication>
#include <QThread>

#include "../LCSServer/MessageBroadcaster.h"

#include "API.h"

API *API::m_instance = NULL;

API::API(const QString &server, const int port, QObject *parent)
    : QObject(parent), m_connected (false), m_server(server), m_port(port), m_notificationSocket(NULL)
{
    if(m_server.endsWith('/') || m_server.endsWith('\\'))
        m_server.chop(1);
    setupUDPSocket();
    if(m_server.length() > 0)
        setupNotificationSocket();
}

API::API(QObject *parent)
    : QObject(parent), m_connected (false), m_notificationSocket(NULL)
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

void API::activateTurnout(int deviceID, int newState)
{
    QUrl url(buildUrl(QString("activate_turnout?deviceID=%1&turnoutState=%2").arg(deviceID).arg(newState)));

    sendToServer(url, QString(), NetActionGet);
}

void API::activateRoute(int routeID)
{
    QUrl url(buildUrl(QString("activate_route?routeID=%1").arg(routeID)));

    sendToServer(url, QString(), NetActionGet);
}

void API::lockRoute(int routeID, bool lock)
{
    QUrl url(buildUrl(QString("lock_route?routeID=%1&lock=%2").arg(routeID).arg(lock)));

    sendToServer(url, QString(), NetActionGet);
}

QString API::getControllerList(int controllerID)
{
    QString json;
    QString s("controller_list");
    if(controllerID > 0)
        s += QString("?controllerID=%1").arg(controllerID);

    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getControllerModuleListByControllerID(int controllerID)
{
    QString json;
    QString s("controller_module_list");
    if(controllerID > 0)
        s.append(QString("?controllerID=%1").arg(controllerID));

    QUrl url(buildUrl(s));
    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getControllerModuleListByModuleID(int controllerModuleID)
{
    QString json;
    QString s("controller_module_list");
    if(controllerModuleID > 0)
        s.append(QString("?controllerModuleID=%1").arg(controllerModuleID));

    QUrl url(buildUrl(s));
    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getDeviceList(int controllerID, int moduleID, DeviceClassEnum deviceType)
{
    QString json;
    QString s("device_list");
    if(deviceType != DeviceUnknown)
        s.append(QString("?classCode=%1").arg(deviceType));
    else if(controllerID > 0)
        s.append(QString("?controllerID=%1").arg(controllerID));
    else if(moduleID > 0)
        s.append(QString("?moduleID=%1").arg(moduleID));
    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getSignalAspectList(int deviceID)
{
    QString json;
    QString s("signal_aspect_list");
    if(deviceID > 0)
        s.append(QString("?deviceID=%1").arg(deviceID));
    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getSignalConditionList(int aspectID)
{
    QString json;
    QString s("signal_condition_list");
    if(aspectID > 0)
        s.append(QString("?aspectID=%1").arg(aspectID));
    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getRouteList()
{
    QString json;
    QString s("route_list");
    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getRouteEntryList(int routeID)
{
    QString json;
    QString s("route_entry_list");
    if(routeID > 0)
        s.append(QString("?routeID=%1").arg(routeID));
    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;}

QString API::getDevicePropertyList(int deviceID)
{
    QString json;
    QString s("device_property_list");
    if(deviceID > 0)
        s.append(QString("?deviceID=%1").arg(deviceID));
    QUrl url(buildUrl(s));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

Entity API::deleteEntity(const Entity &entity)
{
    QJsonObject obj = entity.getObject();
    obj.remove("dbError");
    QJsonDocument doc;
    doc.setObject(obj);
    QString retJson;
    QString s(QString("entity/%1").arg(entity.getEntityName()));
    QUrl url(buildUrl(s));

    retJson = sendToServer(url, doc.toJson(), NetActionDelete);

    QJsonDocument retDoc = QJsonDocument::fromJson(retJson.toLatin1());
    Entity retEntity;
    retEntity.setObject(retDoc.object());

    return retEntity;
}

Entity API::saveEntity(const Entity &entity, bool isNew)
{
    QJsonObject obj = entity.getObject();
    obj.remove("dbError");
    QJsonDocument doc;
    doc.setObject(obj);
    QString retJson;
    QString s(QString("entity/%1").arg(entity.getEntityName()));
    QUrl url(buildUrl(s));

    NetActionType actionType = NetActionUpdate;
    if(isNew)
        actionType = NetActionAdd;
    retJson = sendToServer(url, doc.toJson(), actionType);

    QJsonDocument retDoc = QJsonDocument::fromJson(retJson.toLatin1());
    Entity retEntity;
    retEntity.setObject(retDoc.object());

    return retEntity;
}

bool API::getApiReady()
{
    return m_connected;
}

void API::restartController(int serialNumber)
{
    MessageBroadcaster::instance()->sendResetCommand(serialNumber);
}

void API::sendControllerConfig(int serialNumber)
{
    MessageBroadcaster::instance()->sendResetConfigCommand(serialNumber);
}

void API::sendControllerNotificationList(int serialNumber)
{
    MessageBroadcaster::instance()->sendResetNotificationListCommand(serialNumber);
}

void API::sendDeviceConfig(int deviceID)
{
    MessageBroadcaster::instance()->sendResetDeviceConfigCommand(deviceID);
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
        ControllerStatusEnum status = (ControllerStatusEnum)s.toInt();
        s = obj["pingLength"].toString();
        quint64 pingLength = s.toInt();
        emit controllerChanged(serialNumber, status, pingLength);
    }
    else if(urlText == "/api/notification/device")
    {
        int deviceID = obj["deviceID"].toVariant().toInt();
        int status = obj["deviceState"].toVariant().toInt();
        emit deviceChanged(deviceID, status);
    }
    else if(urlText == "/api/notification/route")
    {
        int routeID = obj["routeID"].toVariant().toInt();
        bool isActive = obj["isActive"].toVariant().toBool();
        bool isLocked = obj["isLocked"].toVariant().toBool();
        bool canLock = obj["canLock"].toVariant().toBool();
        emit routeChanged(routeID, isActive, isLocked, canLock);
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
        m_connected = false;
        emit apiReady();
        QTimer::singleShot(5000, this, SLOT(connectNotificationSocket()));
        break;
    case QAbstractSocket::ConnectedState:
        m_connected = true;
        emit apiReady();
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
            emit serverAddressChanged();
            setupNotificationSocket();
            emit apiReady();
        }
    }
}

void API::findServerSlot()
{
    if(m_server.length() == 0)
    {
        UDPMessage message;
        message.setMessageID(SYS_CONTROLLER_ONLINE);
        MessageBroadcaster::instance()->sendUDPMessage(message);
    }
    else
    {
        m_findServerTimer.stop();
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
    message.setMessageID(SYS_CONTROLLER_ONLINE);
    MessageBroadcaster::instance()->sendUDPMessage(message);
    connect(&m_findServerTimer, SIGNAL(timeout()), this, SLOT(findServerSlot()));
    m_findServerTimer.start(5000);
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
        reply = manager.put(request, json.toLatin1());
    else if(netAction == NetActionAdd)
        reply = manager.post(request, json.toLatin1());
    else if(netAction == NetActionDelete)
        reply = manager.sendCustomRequest(request, "DELETE", json.toLatin1());

    while (true)
    {
        QCoreApplication::processEvents();
        QThread::currentThread()->msleep(100);
        if(reply->isFinished() || reply->isRunning() == false)
            break;
    }

    ret = reply->readAll();
    reply->deleteLater();
    return ret;
}
