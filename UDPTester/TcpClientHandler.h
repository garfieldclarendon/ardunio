#ifndef TCPCLIENTHANDLER_H
#define TCPCLIENTHANDLER_H

#include <QObject>
class QTcpSocket;

class TcpClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpClientHandler(QTcpSocket *clientSocket);

signals:
    void logError(int category, int code, const QString &errorText);

public slots:
    void readyRead(void);

private:

    QTcpSocket *socket;
};

#endif // TCPCLIENTHANDLER_H
