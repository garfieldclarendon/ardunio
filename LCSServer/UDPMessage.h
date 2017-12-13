#ifndef UDPMESSAGE_H
#define UDPMESSAGE_H

#include <QtGlobal>
#include <QString>

#include "GlobalDefs.h"

struct MessageStruct
{
    quint8 startSig[2];
    quint8 messageID;
    long serialNumber;
    quint8 transactionNumber;
    quint8 payload[14];
    quint8 endSig[2];
};

class UDPMessage
{
public:
    UDPMessage(void);
    UDPMessage(const MessageStruct &messageData);

    long getSerialNumber(void) const { return messageSructure.serialNumber; }
    quint8 getMessageID(void) const { return messageSructure.messageID; }
    quint8 getTransactionNumber(void) const { return messageSructure.transactionNumber; }

    void setSerialNumber(int value) { messageSructure.serialNumber = value; }
    void setMessageID(quint8 value) { messageSructure.messageID = value; }
    void setField(quint8 field, quint8 value) { messageSructure.payload[field] = value; }
    void setTransactionNumber(quint8 value) { messageSructure.transactionNumber = value; }

    quint8 getField(int field) const { return messageSructure.payload[field]; }

    const char *getMessageRef(void) const { return (const char *)&messageSructure; }
private:
    MessageStruct messageSructure;
};

#endif // UDPMESSAGE_H
