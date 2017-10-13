#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QJsonObject>

class Message : public QObject
{
    Q_OBJECT
public:
    explicit Message(int serialNumber, const QJsonObject &obj, QObject *parent = 0);
    explicit Message(const Message &other) { copy(other); }

    Message& operator = (const Message &other)
    {
        copy(other);
        return *this;
    }

    int getTransactionID(void) const { return m_transactionID; }
    void setTransactionID(int value) { m_transactionID = value; }
    int getSerialNumber(void) const { return m_serialNumber; }

    void emitMessageSent(void) { emit messageSent(m_transactionID); }
    void emitMessageACKed(void) { emit messageACKed(m_transactionID); }
    void emitErrorSendingMessage(void) { emit errorSendingMessage(m_transactionID); }

signals:
    void messageSent(int transactionID);
    void messageACKed(int transactionID);
    void errorSendingMessage(int transactionID);

public slots:

private:
    void copy(const Message &other);

    int m_transactionID;
    int m_serialNumber;
    QJsonObject m_jsonObject;
};

#endif // MESSAGE_H
