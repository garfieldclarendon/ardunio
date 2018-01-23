#include <QUrl>
#include <QUrlQuery>

#include "APITurnout.h"
#include "MessageBroadcaster.h"
#include "WebServer.h"

APITurnout::APITurnout(QObject *parent) : QObject(parent)
{
    WebServer *webServer = WebServer::instance();

    UrlHandler *handler;

    handler = webServer->createUrlHandler("/api/activate_turnout");
    connect(handler, SIGNAL(handleUrl(APIRequest,APIResponse*)), this, SLOT(handleActivateTurnoutUrl(APIRequest,APIResponse*)), Qt::DirectConnection);
}

void APITurnout::handleActivateTurnoutUrl(const APIRequest &request, APIResponse *)
{
    QUrlQuery urlQuery(request.getUrl());
    int deviceID = urlQuery.queryItemValue("deviceID").toInt();
    TurnoutState newState = (TurnoutState)urlQuery.queryItemValue("turnoutState").toInt();

    UDPMessage message;
    message.setMessageID(TRN_ACTIVATE);
    message.setID(deviceID);
    message.setField(0, newState);

    MessageBroadcaster::instance()->sendUDPMessage(message);
}
