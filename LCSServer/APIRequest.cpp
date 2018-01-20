#include <QTcpSocket>

#include "APIRequest.h"
#include "WebServer.h"
#include "APIResponse.h"

APIRequest::APIRequest()
    : m_actionType(NetActionGet)
{

}

APIRequest::APIRequest(NetActionType actionType, const QUrl url)
    : m_url(url), m_actionType(actionType)
{

}

void APIRequest::copy(const APIRequest &other)
{
    m_url = other.m_url;
    m_payload = other.m_payload;
    m_actionType = other.m_actionType;
}
