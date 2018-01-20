#ifndef APIRESPONSE_H
#define APIRESPONSE_H

#include <QUrl>
#include <QString>
#include <QMetaType>

class APIResponse
{

public:
    APIResponse(void);
    APIResponse(const APIResponse &other) { copy(other); }
    ~APIResponse(void) { }

    QString getReturnCode(void) const {return m_returnCode; }
    void setReturnCode(const QString &value) { m_returnCode = value; }
    QString getContentType(void) const { return m_contentType; }
    void setContenetType(const QString &value) { m_contentType = value; }
    QByteArray getPayload(void) const { return m_payload; }
    void setPayload(const QByteArray &value) { m_payload = value; }

    APIResponse& operator=(const APIResponse &other) { copy(other); return *this; }

private:
    void copy(const APIResponse &other);

    QString m_returnCode;
    QString m_contentType;
    QByteArray m_payload;
};

Q_DECLARE_METATYPE(APIResponse)

#endif // APIRESPONSE_H
