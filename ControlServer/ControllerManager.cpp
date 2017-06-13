#include <QCoreApplication>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaMethod>
#include <QTimer>
#include <QPointer>

#include "ControllerManager.h"
#include "Database.h"
#include "NotificationServer.h"

/*
class ControllerEntry
{
 public:
    explicit ControllerEntry(int serialNumber)
        : m_serialNumber(serialNumber)
    {

    }
    int getSerialNumber(void) const { return m_serialNumber; }
    void setSerialNumber(int value) { m_serialNumber = value; }
    QString getIPAddress(void) const
    {
        QString address;
        if(m_socket)
            address = m_socket->peerAddress().toString();
        return address;
    }
    QWebSocket *getSocket(void) const { return m_socket; }
    void setSocket(QWebSocket *value) { m_socket = value; }

private:
    int m_serialNumber;
    QPointer<QWebSocket> m_socket;
};
*/
ControllerManager * ControllerManager::m_instance = NULL;

ControllerManager::ControllerManager(QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer(QStringLiteral("Controller Server"), QWebSocketServer::NonSecureMode, this)),
      m_pingTimer(NULL), m_transactionID(1)
{
    connect(this, &ControllerManager::sendNotificationMessage, NotificationServer::instance(), &NotificationServer::sendNotificationMessage);
    if (m_server->listen(QHostAddress::Any, UdpPort + 1))
    {
        qDebug() << "Controller listening on port" << (UdpPort + 1);
        connect(m_server, &QWebSocketServer::newConnection,this, &ControllerManager::onNewConnection);
        connect(m_server, &QWebSocketServer::closed, this, &ControllerManager::connectionClosed);
        m_pingTimer = new QTimer(this);
        connect(m_pingTimer, &QTimer::timeout, this, &ControllerManager::pingSlot);
        m_pingTimer->setInterval(30000);
        m_pingTimer->start();
    }
}

ControllerManager::~ControllerManager()
{

}

ControllerManager *ControllerManager::instance()
{
    if(m_instance == NULL)
        m_instance = new ControllerManager(qApp);

    return m_instance;
}

QString ControllerManager::getControllerIPAddress(int serialNumber) const
{
    QString ipAddress;
    for(int x = 0; x < m_socketList.count(); x++)
    {
        if(m_socketList.value(x)->property("serialNumber").toInt() == serialNumber)
            ipAddress = m_socketList.value(x)->peerAddress().toString();
    }
    return ipAddress;
}

bool ControllerManager::sendMessage(int serialNumber, const QJsonObject &obj)
{
    QJsonDocument doc;
    bool ret = true;
    obj["transactionID"] = m_transactionID++;
    doc.setObject(obj);

    QByteArray normalizedSignature = QMetaObject::normalizedSignature("sendMessageSlot(int, QString)");
    int methodIndex = this->metaObject()->indexOfMethod(normalizedSignature);
    QMetaMethod method = this->metaObject()->method(methodIndex);
    method.invoke(this,
                  Qt::QueuedConnection,
                  Q_ARG(int, serialNumber),
                  Q_ARG(QString, QString(doc.toJson())));
    return ret;
}

int ControllerManager::getConnectionSerialNumber(int index) const
{
    int serialNumber(0);
    QWebSocket *socket = m_socketList.value(index);
    if(socket)
        serialNumber = socket->property("serialNumber").toInt();

    return serialNumber;
}

void ControllerManager::getConnectedInfo(int serialNumber, int &version, bool &isOnline)
{
    for(int x = 0; x < m_socketList.count(); x++)
    {
        if(m_socketList.value(x)->property("serialNumber").toInt() == serialNumber)
        {
            version = m_socketList.value(x)->property("version").toInt();
            isOnline = true;
            break;
        }
    }
}

void ControllerManager::sendMessageSlot(int serialNumber, const QString &data)
{
    for(int x = 0; x < m_socketList.count(); x++)
    {
        QWebSocket *socket = m_socketList.value(x);
        if(socket->property("serialNumber").toInt() == serialNumber)
        {
            int count = socket->sendTextMessage(data);
            socket->flush();
            qDebug(QString("ControllerManager::sendMessage length: %1  ret %2 ").arg(data.length()).arg(count).toLatin1());
        }
    }
}

void ControllerManager::onNewConnection(void)
{
    QWebSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QWebSocket::textMessageReceived, this, &ControllerManager::processTextMessage);
    connect(socket, &QWebSocket::disconnected, this, &ControllerManager::connectionClosed);
    connect(this, &ControllerManager::pingSignal, socket, &QWebSocket::ping);
    connect(socket, &QWebSocket::pong, this, &ControllerManager::pongReply);
    m_socketList << socket;
    socket->setProperty("socketTimeout", QDateTime::currentDateTime().toTime_t());
}

void ControllerManager::connectionClosed(void)
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    int serialNumber = socket->property("serialNumber").toInt();
    if(serialNumber > 0)
    {
        bool found = false;
        for(int x = 0; x < m_socketList.count(); x++)
        {
            if(m_socketList.value(x) != socket && m_socketList.value(x)->property("serialNumber").toInt() == serialNumber)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            qDebug(QString("Controller %1 disconnected.").arg(serialNumber).toLatin1());
            emit controllerRemoved(serialNumber);
            createAndSendNotificationMessage(serialNumber, false);
        }
    }
    emit controllerDisconnected(m_socketList.indexOf(socket));
    m_socketList.removeAll(socket);
    socket->deleteLater();
}

void ControllerManager::processTextMessage(QString message)
{
    if(message.startsWith("ACK_"))
    {
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(message.toLatin1());
    QJsonObject root = doc.object();
    QString uri = root["messageUri"].toString();
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    int serialNumber = socket->property("serialNumber").toInt();

    if(uri == "/controller/connect")
    {
        serialNumber = root["serialNumber"].toInt();
        int version = root["version"].toInt();
        socket->setProperty("serialNumber", serialNumber);
        socket->setProperty("version", version);
        bool found = false;
        for(int x = 0; x < m_socketList.count(); x++)
        {
            if(m_socketList.value(x) != socket && m_socketList.value(x)->property("serialNumber").toInt() == serialNumber)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            emit controllerAdded(serialNumber);
            createAndSendNotificationMessage(serialNumber, true);
        }
        emit controllerConnected(m_socketList.indexOf(socket));
   }
    else if(uri == "/controller/multiConfig")
    {
        QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
        sendMultiControllerConfig(serialNumber, socket);
    }
    else
    {
        NetActionType actionType = (NetActionType)root["action"].toInt();
        int moduleIndex = root["moduleIndex"].toInt();
        ClassEnum classCode = (ClassEnum)(int)root["class"].toInt();

        emit newMessage(serialNumber, moduleIndex, classCode, actionType, uri, root);
    }
}

void ControllerManager::sendControllerInfo(int serialNumber, QWebSocket *socket)
{
    qDebug(QString("handleController: controller %1").arg(serialNumber).toLatin1());
    QString controllerName;
    int controllerID;
    Database db;
    db.getControllerIDAndName(serialNumber, controllerID, controllerName);
    QJsonDocument jsonDoc;
    QJsonObject obj;
    obj["messageUri"] = "/controller/name";
    obj["action"] = (int)NetActionUpdate;
    obj["controllerID"] = controllerID;
    obj["controllerName"] = controllerName;
    QString sql;
    sql = QString("SELECT device.id as deviceID, deviceName FROM  device JOIN controllerModule ON device.controllerModuleID = controllerModule.id  WHERE controllerID = %1").arg(controllerID);
    QJsonArray a = db.fetchItems(sql);
    obj["devices"] = a;
    jsonDoc.setObject(obj);
    socket->sendTextMessage(jsonDoc.toJson());
    qDebug(jsonDoc.toJson());
}

void ControllerManager::sendMultiControllerConfig(int serialNumber, QWebSocket *socket)
{
    qDebug(QString("sendMultiControllerConfig: controller %1").arg(serialNumber).toLatin1());

    Database db;
    QString returnPayload = db.getMultiControllerConfig(serialNumber);
    socket->sendTextMessage(returnPayload);
    qDebug(returnPayload.toLatin1());
}

void ControllerManager::pongReply(quint64 length, const QByteArray &)
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if(socket)
    {
        socket->setProperty("socketTimeout", QDateTime::currentDateTime().toTime_t());
        QString txt = QString("Pong reply from %1  Total time: %2").arg(socket->peerAddress().toString()).arg(length);
        qDebug(txt.toLatin1());
        emit controllerPing(m_socketList.indexOf(socket), length);
    }
}

void ControllerManager::pingSlot()
{
    for(int x = 0; x < m_socketList.count(); x++)
    {
        uint timeout = m_socketList.value(x)->property("socketTimeout").toUInt();
        timeout = QDateTime::currentDateTime().toTime_t() - timeout;
        if(timeout > 60)
            m_socketList.value(x)->close();
    }
    QByteArray data;
    emit pingSignal(data);
}

void ControllerManager::createAndSendNotificationMessage(int serialNumber, bool isOnline)
{
    QString uri("/api/notification/controller");
    QJsonObject obj;
    obj["serialNumber"] = serialNumber;
    obj["isOnline"] = isOnline;

    emit sendNotificationMessage(uri, obj);
}
