#ifndef APIREQUEST_H
#define APIREQUEST_H

#include <QUrl>
#include <QByteArray>
#include <QMetaType>

#include "GlobalDefs.h"

/// APIRequest
/// \brief  Contains information for a API web request.
///
/// This class contains all information as it relates to a REST API web request.
class APIRequest
{
public:
    /// Contstructor
    APIRequest(void);
    /// Contstructor
    /// @param NetActionType HTTP action type
    /// @url QUrl Full url of the request including parameters
    APIRequest(NetActionType actionType, const QUrl url);
    /// Copy Contstructor
    APIRequest(const APIRequest &other) { copy(other); }

    /// Returns the HTTP action type
    NetActionType getNetActionType(void) const { return m_actionType; }
    /// Returns the requesting URL
    QUrl getUrl(void) const { return m_url; }
    /// Access function for a message payload.  Usually a JSON document.
    QByteArray getPayload(void) const { return m_payload; }
    /// Sets the message payload.
    void setPayload(const QByteArray &value) { m_payload = value; }

    /// Assignment operator
    APIRequest& operator=(const APIRequest &other) { copy(other); return *this; }

private:
    /// updates the objects internal data to that of "other".
    void copy(const APIRequest &other);

    QUrl m_url;
    NetActionType m_actionType;
    QByteArray m_payload;
};

Q_DECLARE_METATYPE(APIRequest)

#endif // APIREQUEST_H
