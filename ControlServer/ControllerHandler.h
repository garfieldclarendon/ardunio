#ifndef CONTROLLERHANDLER_H
#define CONTROLLERHANDLER_H

#include <QObject>
#include "GlobalDefs.h"

class ControllerHandler : public QObject
{
    Q_OBJECT
public:
    explicit ControllerHandler(QObject *parent = 0);

signals:

public slots:
    void handleUrl(NetActionType actionType, const QUrl &url, const QString &payload, QString &returnPayload);
    void handleConfigUrl(NetActionType actionType, const QUrl &url, const QString &payload, QString &returnPayload);

};

#endif // CONTROLLERHANDLER_H
