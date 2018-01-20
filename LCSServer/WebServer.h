#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QMap>
#include <QSslError>

#include "GlobalDefs.h"
#include "APIRequest.h"
#include "APIResponse.h"

class QUrl;
class QNetworkReply;
class UDPMessage;

class UrlHandler : public QObject
{
    Q_OBJECT

public:
    explicit UrlHandler(QObject *parent)
        : QObject(parent)
    {

    }

    APIResponse handleRequest(const APIRequest &request)
    {
        APIResponse response;
        emit handleUrl(request, &response);

        return response;
    }

signals:
    void handleUrl(const APIRequest &request, APIResponse *response);
};

class WebServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit WebServer(QObject *parent = 0);

    void incomingConnection(int socket);
    void startServer(quint16 port);
    void handleUdpMessage(NetActionType action, const QString &path, const QString &payload);

    static WebServer *instance(void);
    static QString createHeader(const QString &httpCode, int bodySize, const QString &contentType = "text/html");

signals:

public slots:
    UrlHandler *createUrlHandler(const QString &path);
    UrlHandler *getUrlHandler(const QString &path);
    bool sendMessage(NetActionType actionType, const QString &uri, const QString &payload, QString &returnPayload);

private:
    QMap<QString, UrlHandler *> m_handlerMap;

    static WebServer *m_instance;
};

#endif // TCPSERVER_H
