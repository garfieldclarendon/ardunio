#include <QUrl>
#include <QUrlQuery>
#include <QJsonArray>
#include <QJsonDocument>

#include "APISignal.h"
#include "MessageBroadcaster.h"
#include "WebServer.h"
#include "Database.h"

APISignal::APISignal(QObject *parent) : QObject(parent)
{
    WebServer *webServer = WebServer::instance();

    UrlHandler *handler;

    handler = webServer->createUrlHandler("/api/signal_aspect_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetSignalAspectList(APIRequest,APIResponse*)), Qt::DirectConnection);
    handler = webServer->createUrlHandler("/api/signal_condition_list");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleGetSignalConditionList(APIRequest,APIResponse*)), Qt::DirectConnection);
}

void APISignal::handleGetSignalAspectList(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    qDebug(QString("handleGetSignalAspectList.").toLatin1());
    Database db;

    QString sql = QString("SELECT id as signalAspectID, deviceID, sortIndex, redMode, yellowMode, GreenMode FROM signalAspect");
    sql += QString(" WHERE deviceID = %1").arg(deviceID);
    sql += QString(" ORDER BY sortIndex");

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}

void APISignal::handleGetSignalConditionList(const APIRequest &request, APIResponse *response)
{
    QUrlQuery urlQuery(request.getUrl());
    int aspectID = urlQuery.queryItemValue("aspectID").toInt();
    qDebug(QString("handleGetSignalConditionList.").toLatin1());
    Database db;

    QString sql = QString("SELECT id as signalConditionID, signalAspectID, deviceID, conditionOperand, deviceState FROM signalCondition");
    sql += QString(" WHERE signalAspectID = %1").arg(aspectID);

    QJsonArray jsonArray = db.fetchItems(sql);

    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray data(doc.toJson());

    response->setPayload(data);
}
