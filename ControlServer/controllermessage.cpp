#include "controllermessage.h"

int ControllerMessage::m_transactionID = 0;

ControllerMessage::ControllerMessage(int serialNumber, const QJsonObject &obj, QObject *parent)
    : QObject(parent), m_serialNumber(serialNumber), m_jsonObject(obj)
{
    m_transactionID++;
}

ControllerMessage::ControllerMessage()
    : QObject(NULL), m_serialNumber(-1)
{

}

void ControllerMessage::copy(const ControllerMessage &other)
{
    m_transactionID = other.m_transactionID;
    m_serialNumber = other.m_serialNumber;
    m_jsonObject = other.m_jsonObject;
}

