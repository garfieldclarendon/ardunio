#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "API.h"

API::API(const QString &server, const int port, QObject *parent)
    : QObject(parent), m_server(server), m_port(port), m_notificationSocket(NULL)
{
    if(m_server.endsWith('/') || m_server.endsWith('\\'))
        m_server.chop(1);
    setupNotificationSocket();
}

API::~API()
{
    if(m_notificationSocket)
        m_notificationSocket->deleteLater();
}

QString API::getControllerList()
{
    QString json;
    QUrl url(buildUrl("controller_list"));

    json = sendToServer(url, QString(), NetActionGet);

    return json;
}

QString API::getDeviceList()
{
    QString json;
    return json;
}

void API::textMessageReceived(const QString &message)
{
    QJsonDocument doc(QJsonDocument::fromJson(message.toLatin1()));
    QJsonObject obj = doc.object();
    QString urlText = obj["uri"].toString();

    if(urlText == "/api/notification/controller")
    {
        int serialNumber = obj["serialNumber"].toInt();
        ControllerStatus status = (ControllerStatus)obj["status"].toInt();
        emit controllerChanged(serialNumber, status);
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

void API::setupNotificationSocket()
{
    if(m_server.length() > 0 && m_port > 0)
    {
        m_notificationSocket = new QWebSocket;
        connect(m_notificationSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageReceived(QString)));
        connect(m_notificationSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(notificationStateChanged(QAbstractSocket::SocketState)));

        connectNotificationSocket();
    }
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

    while(true)
    {
        reply->waitForReadyRead(200);
        if(reply->bytesAvailable() >= reply->header(QNetworkRequest::ContentLengthHeader).toInt())
        {
            ret = reply->readAll();
            break;
        }
    }

    return ret;
}
