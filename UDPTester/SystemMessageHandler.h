#ifndef SYSTEMMESSAGEHANDLER_H
#define SYSTEMMESSAGEHANDLER_H

#include <QObject>

class UDPMessage;

class SystemMessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit SystemMessageHandler(QObject *parent = 0);

signals:

public slots:
    void handleMessage(const UDPMessage &message);

private:
    void registerController(const UDPMessage &message);
    void createNewMacAddress(quint8 *mac);
};

#endif // SYSTEMMESSAGEHANDLER_H
