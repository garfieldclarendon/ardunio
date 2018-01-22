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

/// UrlHandler
/// \brief  A proxy class that connects the web server with the handler of the request.
///
/// This class acts as a proxy betweent he web server and the object handles the request.  \ref WebServer "WebServer" creates an
/// instance of this class by calling WebServer::createUrlHandler for a given url.  The web server calls the handleRequest member
/// which emits the handleUrl signal.
class UrlHandler : public QObject
{
    friend class WebServer;
    Q_OBJECT
    /// Private Constructor
    explicit UrlHandler(QObject *parent)
        : QObject(parent)
    {

    }

public:
    /// Raises the handleUrl signal.  The reference to the APIResponse object gets filled with the information to be returned
    /// to the caller.  This implies that the signal cannot be handled by a Qt::QueuedConnection.
    APIResponse handleRequest(const APIRequest &request)
    {        
        APIResponse response;
        emit handleUrl(request, &response);

        return response;
    }

signals:
    /// Objects connect to this signal to handle the API request.
    void handleUrl(const APIRequest &request, APIResponse *response);
};

/// WebServer
/// \brief  An HTTP web server implementation that provides the REST API.
///
/// This class creates an HTTP web server based on a QTcpServer.  Upon a tcp connection, the class looks for a url handler registered
/// through createUrlHandler.  If found, the handler's handleRequest member is called.
/// A \ref UrlHandler instance is created by calling WebServer::createUrlHandler for a given url.  The web server calls the handleRequest member
/// which emits the handleUrl signal.
class WebServer : public QTcpServer
{
    Q_OBJECT

public:
    /// Constructor
    explicit WebServer(QObject *parent = 0);
    /// This virtual function is called by QTcpServer when a new connection is available. The socketDescriptor argument is the native socket descriptor for the accepted connection.
    /// A new \ref WebServerThread object is created which handles the socket request.
    void incomingConnection(int socket);
    /// Starts the server; listening for incoming connections of the provided port.
    void startServer(quint16 port);
    /// Returns/creates the singleton instance
    static WebServer *instance(void);
    /// Creates a standard HTTP header.
    static QString createHeader(const QString &httpCode, int bodySize, const QString &contentType = "text/html");

signals:

public slots:
    /// Creates a new \ref UrlHandler object and registers the handler for the provided path.
    UrlHandler *createUrlHandler(const QString &path);
    /// Returns the \ref UrlHandler object for the given path.  NULL is returned if a handler for the path is not found
    UrlHandler *getUrlHandler(const QString &path);

private:
    QMap<QString, UrlHandler *> m_handlerMap;

    static WebServer *m_instance;
};

#endif // TCPSERVER_H
