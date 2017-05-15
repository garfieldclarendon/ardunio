#ifndef WEBSERVERTHREAD_H
#define WEBSERVERTHREAD_H

#include <QThread>

#include "GlobalDefs.h"
#include "APIHandler.h"

class WebServerThread : public QThread
{
    Q_OBJECT
public:
    WebServerThread(int socketDescriptor, QObject *parent);
    ~WebServerThread(void);

    void run() override;

public slots:
    void readClient(void);

private:
    void handleDownloadFirmware(QTcpSocket* socket, const QUrl &url);
    void handleSocket(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload);
    QByteArray getFile(const QString &fileName);

    int socketDescriptor;
    APIHandler m_apiHandler;
};

#endif // WEBSERVERTHREAD_H
