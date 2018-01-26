#ifndef APIRESPONSE_H
#define APIRESPONSE_H
#include <QMetaType>

#include <QUrl>
#include <QString>

/// APIResponse
/// \brief  Contains information for an API web response.
///
/// This class contains all information as it relates to a REST API web response.
class APIResponse
{

public:
    /// Consturctor
    APIResponse(void);
    /// Copy Constructor
    APIResponse(const APIResponse &other) { copy(other); }
    /// Destructor
    ~APIResponse(void) { }

    /// Returns the HTTP return code.  By default, the return code is set to "200 OK".
    QString getReturnCode(void) const {return m_returnCode; }
    /// Sets the HTTP return code that will be returned to the requestor.
    void setReturnCode(const QString &value) { m_returnCode = value; }
    /// Returns the HTTP content type.  By default, the content type is set to "text/html".
    QString getContentType(void) const { return m_contentType; }
    /// Sets the HTTP content type.
    void setContenetType(const QString &value) { m_contentType = value; }
    /// Returns the payload
    QByteArray getPayload(void) const { return m_payload; }
    /// Sets the payload that is returned to the requestor.
    void setPayload(const QByteArray &value) { m_payload = value; }

    /// Assignment operator
    APIResponse& operator=(const APIResponse &other) { copy(other); return *this; }

private:
    /// updates the objects internal data to that of "other".
    void copy(const APIResponse &other);

    QString m_returnCode;
    QString m_contentType;
    QByteArray m_payload;
};

Q_DECLARE_METATYPE(APIResponse)

#endif // APIRESPONSE_H
