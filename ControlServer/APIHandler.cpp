#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlQuery>

#include "GlobalDefs.h"
#include "APIHandler.h"
#include "DeviceManager.h"
#include "TurnoutHandler.h"
#include "RouteHandler.h"
#include "Database.h"
#include "WebServer.h"
#include "MessageBroadcaster.h"
#include "ControllerManager.h"

APIHandler::APIHandler(QObject *parent)
    : QObject(parent)
{

}

void APIHandler::handleClient(QTcpSocket *socket, const QString &path, const QString &actionText, const QString &payload)
{
    QUrl url(path);

    if(url.path().contains("activate_turnout"))
    {
        QString result = QString("HTTP/1.0 200 OK\r\n"
                                              "\r\n");

        qDebug("SEND OK");
        socket->write(result.toLatin1());
        socket->flush();
        socket->close();

        handleActivateTurnout(socket, url, actionText, payload);
    }
    else if(url.path().contains("activate_route"))
    {
        QString result = QString("HTTP/1.0 200 OK\r\n"
                                              "\r\n");

        qDebug("SEND OK");
        socket->write(result.toLatin1());
        socket->flush();
        socket->close();

        handleActivateRoute(socket, url, actionText, payload);
    }
    else if(url.path().contains("panel_routes"))
    {
        handleGetPanelRouteList(socket, url);
    }
    else if(url.path().contains("panel_list"))
    {
        handleGetPanelList(socket, url);
    }
    else if(url.path().contains("device_list"))
    {
        handleGetDeviceList(socket, url);
    }
    else if(url.path().contains("controller_list"))
    {
        handleGetControllerList(socket, url);
    }
    else if(url.path().contains("send_module_config"))
    {
        handleSendModuleConfig(socket, url);
    }
    else if(url.path().contains("send_controller_firmware"))
    {
        handleControllerFirmwareUpdate(socket, url);
    }
    else if(url.path().contains("send_controller_reset_config"))
    {
        handleControllerResetConfig(socket, url);
    }
    else if(url.path().contains("send_controller_reset"))
    {
        handleControllerReset(socket, url);
    }
    else
    {
        QString result = QString("HTTP/1.0 200 OK\r\n"
                                              "\r\n");

        qDebug("SEND OK");
        socket->write(result.toLatin1());
        socket->flush();
        socket->close();
    }
}

void APIHandler::handleActivateTurnout(QTcpSocket *socket, const QUrl &url, const QString &, const QString &)
{
    QString result = QString("HTTP/1.0 200 OK\r\n"
                                          "\r\n");

    qDebug("SEND OK");
    socket->write(result.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    TurnoutState newState = (TurnoutState)urlQuery.queryItemValue("turnoutState").toInt();

    if((newState == TrnNormal || newState == TrnDiverging))
    {
        TurnoutHandler *turnoutHandler = qobject_cast<TurnoutHandler *>(DeviceManager::instance()->getHandler(ClassTurnout));
        if(turnoutHandler)
            turnoutHandler->activateTurnout(deviceID, newState);
    }
    else
    {
        // TODO:
        // Probably should return an error here
    }
}

void APIHandler::handleActivateRoute(QTcpSocket *socket, const QUrl &url, const QString &, const QString &)
{
    QString result = QString("HTTP/1.0 200 OK\r\n"
                                          "\r\n");

    qDebug("SEND OK");
    socket->write(result.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int routeID = urlQuery.queryItemValue("routeID").toInt();

    if(routeID > 0)
    {
        RouteHandler::instance()->activateRoute(routeID);
    }
    else
    {
        // TODO:
        // Probably should return an error here
    }
}

void APIHandler::handleGetPanelRouteList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int panelID = urlQuery.queryItemValue("panelID").toInt();

    qDebug(QString("handleGetPanelRouteList.  PanelID = %1").arg(panelID).toLatin1());
    Database db;

    QString sql = QString("SELECT ID as routeID, routeName, routeDescription FROM Route WHERE routeID IN (SELECT inputID FROM PanelInputEntry WHERE panelModuleID = %1) ORDER BY routeID").arg(panelID);
    QJsonArray jsonArray = db.fetchItems(sql);

    for(int x = 0; x < jsonArray.size(); x++)
    {
        QJsonObject obj = jsonArray[x].toObject();
        QString routeID = obj["routeID"].toString();
        bool isActive = RouteHandler::instance()->isRouteActive(routeID.toInt());
        obj["isActive"] = isActive;
        jsonArray[x] = obj;
    }

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();

}

void APIHandler::handleGetPanelList(QTcpSocket *socket, const QUrl &)
{
    qDebug(QString("handleGetPanelList.  SerialNumber = %1 ModuleIndex = %2").toLatin1());
    Database db;

    QString sql = QString("SELECT controllerModule.id as panelID, moduleName as panelName, controllerModule.moduleIndex, serialNumber FROM controllerModule JOIN controller ON controller.id = controllerModule.controllerID WHERE moduleClass = 2 ORDER BY moduleName");

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();}

void APIHandler::handleGetDeviceList(QTcpSocket *socket, const QUrl &url)
{
    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
    int moduleIndex = -1;
    int classCode = urlQuery.queryItemValue("classCode").toInt();
    if(urlQuery.hasQueryItem("moduleIndex"))
        moduleIndex = urlQuery.queryItemValue("moduleIndex").toInt();

    qDebug(QString("handleGetDeviceList.  SerialNumber = %1 ModuleIndex = %2").arg(serialNumber).arg(moduleIndex).toLatin1());
    Database db;

    QString sql = QString("SELECT device.id as deviceID, deviceName, deviceDescription, device.moduleIndex as port, controllerModule.moduleIndex, moduleClass, serialNumber FROM device JOIN controllerModule ON device.controllerModuleID = controllerModule.id JOIN controller ON controller.id = controllerModule.controllerID");

    QString where(" WHERE ");
    bool useAnd = false;
    if(serialNumber > 0)
    {
        useAnd = true;
        where += QString("serialNumber = %1 ").arg(serialNumber);
    }
    if(moduleIndex > -1)
    {
        if(useAnd)
            where += QString("AND moduleIndex = %1 ").arg(moduleIndex);
        else
            where += QString("moduleIndex = %1 ").arg(moduleIndex);

        useAnd = true;
    }
    if(classCode > 0)
    {
        if(useAnd)
            where += QString("AND moduleClass = %1 ").arg(classCode);
        else
            where += QString("moduleClass = %1 ").arg(classCode);

        useAnd = true;
    }
    if(useAnd)
        sql += where;
    sql += QString(" ORDER BY deviceName");

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleGetControllerList(QTcpSocket *socket, const QUrl & /*url*/)
{
    qDebug(QString("handleGetModuleList.").toLatin1());
    Database db;

    QString sql = QString("SELECT serialNumber, id as controllerID, controllerName, controllerDescription FROM controller");
    sql += QString(" ORDER BY controllerName");

    QJsonArray jsonArray = db.fetchItems(sql);

    for(int x = 0; x < jsonArray.size(); x++)
    {
        QJsonObject obj = jsonArray[x].toObject();
        QString serialNumber = obj["serialNumber"].toString();
        int version = 0;
        ControllerStatus status = ControllerUnknown;
        ControllerManager::instance()->getConnectedInfo(serialNumber.toInt(), version, status);
        obj["status"] = status;
        obj["version"] = version;

        jsonArray[x] = obj;
    }


    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    QString header = WebServer::createHeader("200 OK", data.size());

    socket->write(header.toLatin1());
    socket->write(data);
    socket->flush();
    socket->close();
}

void APIHandler::handleSendModuleConfig(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();
    int moduleIndex = urlQuery.queryItemValue("moduleIndex").toInt();
    qDebug(QString("handleGetPanelList.  SerialNumber = %1 ModuleIndex = %2").toLatin1());
    Database db;

    QString sql = QString("SELECT controllerModule.moduleClass, serialNumber FROM controllerModule JOIN controller ON controller.id = controllerModule.controllerID WHERE serialNumber = %1 AND moduleIndex = %2").arg(serialNumber).arg(moduleIndex);
    QSqlQuery query = db.executeQuery(sql);
    while(query.next())
    {
        int moduleClass = query.value("moduleClass").toInt();
        if(moduleClass == ClassTurnout)
        {
            TurnoutHandler *turnoutHandler = qobject_cast<TurnoutHandler *>(DeviceManager::instance()->getHandler(ClassTurnout));
            if(turnoutHandler)
            {
                turnoutHandler->sendConfig(serialNumber, moduleIndex);
            }
        }
    }
}

void APIHandler::handleControllerFirmwareUpdate(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendDownloadFirmware(serialNumber);
}

void APIHandler::handleControllerReset(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetCommand(serialNumber);
}

void APIHandler::handleControllerResetConfig(QTcpSocket *socket, const QUrl &url)
{
    QString header = WebServer::createHeader("200 OK", 0);

    socket->write(header.toLatin1());
    socket->flush();
    socket->close();

    QUrlQuery urlQuery(url);
    int serialNumber = urlQuery.queryItemValue("serialNumber").toInt();

    MessageBroadcaster::instance()->sendResetConfigCommand(serialNumber);
}

