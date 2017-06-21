#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <QObject>

class QTcpSocket;
class QUrl;

class APIHandler : public QObject
{
    Q_OBJECT
public:
    explicit APIHandler(QObject *parent = 0);

    void handleClient(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload);

signals:

public slots:
private:
    void handleActivateTurnout(QTcpSocket *socket, const QUrl &url, const QString &actionText, const QString &payload);
    void handleActivateRoute(QTcpSocket *socket, const QUrl &url, const QString &actionText, const QString &payload);
    void handleGetPanelRouteList(QTcpSocket *socket, const QUrl &url);
    void handleGetPanelList(QTcpSocket *socket, const QUrl &url);
    void handleGetDeviceList(QTcpSocket *socket, const QUrl &url);
    void handleGetControllerList(QTcpSocket *socket, const QUrl &url);
    void handleSendModuleConfig(QTcpSocket *socket, const QUrl &url);
    void handleControllerFirmwareUpdate(QTcpSocket *socket, const QUrl &url);
    void handleControllerReset(QTcpSocket *socket, const QUrl &url);
    void handleControllerResetConfig(QTcpSocket *socket, const QUrl &url);
};

#endif // APIHANDLER_H
