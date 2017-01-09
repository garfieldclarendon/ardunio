#include <QWebSocket>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "JMRIWebSocket.h"
#include "Database.h"
#include "MessageBroadcaster.h"

JMRIWebSocket::JMRIWebSocket(const QUrl &url, QObject *parent) : QObject(parent),
    m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this)), m_url(url), m_ignoreMessage(false)
{
    qDebug() << "WebSocket server:" << url;
    connect(m_webSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_webSocket, SIGNAL(disconnected()), this, SLOT(closed()));
    connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    connectToJMRI();

//    testPost();

    connect(MessageBroadcaster::instance(), SIGNAL(newMessage(UDPMessage)), this, SLOT(onNewMessage(UDPMessage)));
    connect(&m_heartbeatTimer, SIGNAL(timeout()), this, SLOT(heatbeatTimerProc()));
    m_heartbeatTimer.start(5000);
}

void JMRIWebSocket::connectToJMRI()
{
    qDebug() << "Connecting to: " << m_url;
    m_webSocket->open(m_url);
}

void JMRIWebSocket::onNewMessage(const UDPMessage &message)
{
    qDebug() << "onNewMessage:" << message.getMessageID();

    if(message.getMessageID() == TRN_STATUS)
    {
        if(message.getIntValue1() > 0 && (message.getByteValue1() == TrnNormal || message.getByteValue1() == TrnDiverging))
        {
            if(m_turnoutMap.value(message.getIntValue1()) != message.getByteValue1())
            {
                Database db;
                QString name = db.getTurnoutName(message.getIntValue1());
                sendTurnoutStatusMessage(name, (TurnoutState)message.getByteValue1());
                m_turnoutMap[message.getIntValue1()] = message.getByteValue1();
            }
        }
        if(message.getIntValue2() > 0 && (message.getByteValue2() == TrnNormal || message.getByteValue2() == TrnDiverging))
        {
            if(m_turnoutMap.value(message.getIntValue2()) != message.getByteValue2())
            {
                Database db;
                QString name = db.getTurnoutName(message.getIntValue2());
                sendTurnoutStatusMessage(name, (TurnoutState)message.getByteValue2());
                m_turnoutMap[message.getIntValue2()] = message.getByteValue2();
            }
        }
    }
}

void JMRIWebSocket::onConnected()
{
    qDebug() << "WebSocket connected";

    m_turnoutMap.clear();

    connect(m_webSocket, SIGNAL(textMessageReceived(QString)),
            this, SLOT(processTextMessage(QString)));
}

void JMRIWebSocket::processTextMessage(const QString message)
{
    qDebug() << "processTextMessage";
    qDebug() << message;

    QJsonDocument doc = QJsonDocument::fromJson(message.toLatin1());
    QJsonObject command = doc.object();

    if(command.empty() == false && command["type"] == "turnout")
    {
        QJsonObject turnout = command["data"].toObject();
        if(turnout.empty() == false)
        {
            Database db;
            int turnoutID = db.getTurnoutID(turnout["userName"].toString());
            if(turnoutID > 0)
            {
                TurnoutState newState(TrnUnknown);
                if(turnout["state"].toInt() == 2)
                    newState = TrnNormal;
                else if(turnout["state"].toInt() == 4)
                    newState = TrnDiverging;

                if(newState != TrnUnknown && m_turnoutMap.value(turnoutID) != newState)
                {
                    UDPMessage message;
                    message.setMessageClass(ClassTurnout);
                    message.setLValue(123);
                    message.setByteValue1(newState);
                    message.setIntValue1(turnoutID);
                    message.setMessageID(TRN_ACTIVATE);

                    m_ignoreMessage = true;
                    m_turnoutMap[turnoutID] = newState;
                    emit newMessage(message);
                }
            }
        }
    }
}

void JMRIWebSocket::processBinaryMessage(const QByteArray message)
{
    qDebug() << "processBinaryMessage";
}

void JMRIWebSocket::closed()
{
    qDebug() << "socketDisconnected";
}

void JMRIWebSocket::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    qDebug() << "onError" << m_webSocket->errorString();
}

void JMRIWebSocket::heatbeatTimerProc()
{
    if(m_webSocket->state() == QAbstractSocket::ConnectedState)
    {
        QJsonDocument doc;
        QJsonObject obj;
        obj["type"] = "ping";
        doc.setObject(obj);

        QString s = doc.toJson(QJsonDocument::Compact);

        m_webSocket->sendTextMessage(s);
    }
    else
    {
        connectToJMRI();
    }
}

void JMRIWebSocket::sendTurnoutStatusMessage(const QString &name, TurnoutState currentState)
{
    QJsonDocument doc;
    QJsonObject obj;
    QJsonObject turnout;
    obj["type"] = "turnout";
    turnout["name"] = name;
    turnout["test"] = "hello";
    if(currentState == TrnNormal)
        turnout["state"] = "2";
    else if(currentState == TrnDiverging)
        turnout["state"] = "4";
    obj["data"] = turnout;
    doc.setObject(obj);
    m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
}

void JMRIWebSocket::testPost()
{
    QJsonDocument doc;
    QJsonObject obj;
    QJsonObject turnout;
    obj["test"] = "hello";
    obj["type"] = "turnout";
//    turnout["name"] = "ITTST-4";
    turnout["userName"] = "TST-4";
    turnout["test"] = "hello";
    turnout["state"] = "4";
    obj["data"] = turnout;
    doc.setObject(obj);

    QNetworkRequest request;
    request.setUrl(QUrl("http://127.0.0.1:12080/json/turnout/ITTST-4/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
//    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->put(request, doc.toJson(QJsonDocument::Compact));
    connect(reply, SIGNAL(readyRead()), this, SLOT(testPostReadyRead()));
//    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
//            this, SLOT(slotError(QNetworkReply::NetworkError)));
//    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
//            this, SLOT(slotSslErrors(QList<QSslError>)));
}

void JMRIWebSocket::testPostReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QString s = reply->readAll();
    qDebug(s.toLatin1());
}
