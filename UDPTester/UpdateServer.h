#ifndef UPDATESERVER_H
#define UPDATESERVER_H

#include <QTcpServer>

class UpdateServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit UpdateServer(quint16 port, QObject *parent = 0);

    void incomingConnection(int socket);

signals:

public slots:
    void readClient(void);
    void discardClient(void);

private:
    QString createHeader(const QString &httpCode, int bodySize);
    QByteArray getFile(const QString &fileName);

    int m_contentLength;
};

#endif // UPDATESERVER_H
