#ifndef APIREQUEST_H
#define APIREQUEST_H

#include <QUrl>
#include <QByteArray>
#include <QMetaType>

#include "GlobalDefs.h"

class APIResponse;
class QTcpSocket;

class APIRequest
{
public:
    APIRequest(void);
    APIRequest(NetActionType actionType, const QUrl url);
    APIRequest(const APIRequest &other) { copy(other); }

    NetActionType getNetActionType(void) const { return m_actionType; }
    QUrl getUrl(void) const { return m_url; }
    QByteArray getPayload(void) const { return m_payload; }
    void setPayload(const QByteArray &value) { m_payload = value; }

    APIRequest& operator=(const APIRequest &other) { copy(other); return *this; }

private:
    void copy(const APIRequest &other);

    QUrl m_url;
    NetActionType m_actionType;
    QByteArray m_payload;
};

Q_DECLARE_METATYPE(APIRequest)

#endif // APIREQUEST_H
