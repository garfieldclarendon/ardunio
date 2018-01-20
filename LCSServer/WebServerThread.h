#ifndef WEBSERVERTHREAD_H
#define WEBSERVERTHREAD_H

#include <QThread>

#include "GlobalDefs.h"
#include "APIHandler.h"
#include "APIResponse.h"

class WebServerThread : public QThread
{
    Q_OBJECT
public:
    WebServerThread(int socketDescriptor, QObject *parent);
    ~WebServerThread(void);

    void run();

private:
    void handleSocket(const QString &path, const QString &actionText, const QString &payload, APIResponse &response);
    QByteArray getFile(const QString &fileName);

    int socketDescriptor;
    APIHandler m_apiHandler;
};

#endif // WEBSERVERTHREAD_H
