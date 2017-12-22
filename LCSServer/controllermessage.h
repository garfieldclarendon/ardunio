#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>
#include <QJsonObject>

class ControllerMessage : public QObject
{
    Q_OBJECT
public:
    explicit ControllerMessage(int serialNumber, const QJsonObject &obj, QObject *parent = 0);
    ControllerMessage(const ControllerMessage &other) : QObject(NULL) { copy(other); }
    ControllerMessage(void);

    ControllerMessage& operator = (const ControllerMessage &other)
    {
        copy(other);
        return *this;
    }

    int getID(void) const { return m_transactionID; }
    int getSerialNumber(void) const { return m_serialNumber; }
    QJsonObject getObject(void) const { return m_jsonObject; }
    int getTransactionNumber(void) const { return m_transactionID; }

signals:

public slots:

private:
    void copy(const ControllerMessage &other);

    int m_serialNumber;
    QJsonObject m_jsonObject;
    static int m_transactionID;
};

#endif // MESSAGE_H
