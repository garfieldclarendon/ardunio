#ifndef WEBSERVERTHREAD_H
#define WEBSERVERTHREAD_H

#include <QThread>

#include "GlobalDefs.h"
#include "APIResponse.h"

/// WebServerThread
/// \brief  Handles a HTTP server socket request on a separate thread.
///
/// This class creates a QTcpSocket for a given socket descriptor.  The run member creates the socket and parses the incoming HTTP header
/// and attempts to find a \ref UrlHandler for the url specified in the HTTP header.
class WebServerThread : public QThread
{
    Q_OBJECT
public:
    /// Constructor
    WebServerThread(int m_socketDescriptor, QObject *parent);
    /// Descructor
    ~WebServerThread(void);
    /// This virtual function is called by QThread when the thread is started.
    void run();

private:
    void handleSocket(const QString &path, const QString &actionText, const QString &payload, APIResponse &response);
    QByteArray getFile(const QString &fileName);

    int m_socketDescriptor;
};

#endif // WEBSERVERTHREAD_H
