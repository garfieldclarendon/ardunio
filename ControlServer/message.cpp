#include "message.h"

Message::Message(int serialNumber, const QJsonObject &obj, QObject *parent)
    : QObject(parent), m_transactionID(-1), m_serialNumber(serialNumber), m_jsonObject(obj)
{

}

void Message::copy(const Message &other)
{
    m_transactionID = other.m_transactionID;
    m_serialNumber = other.m_serialNumber;
    m_jsonObject = other.m_jsonObject;
}

