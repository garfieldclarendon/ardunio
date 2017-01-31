#ifndef UDPMESSAGE_H
#define UDPMESSAGE_H

#include <QMap>
#include "GlobalDefs.h"
/*
union PayloadUnion
{
    quint8 payload[10];
    struct PayloadSruct
    {
        qint32 lValue;
        qint16 intValue1;
        qint16 intValue2;
        quint8 byteValue1;
        quint8 byteValue2;
    } payloadStruct;
};
*/
struct DeviceStatusStruct
{
    short id;
    quint8 status;
};

union PayloadUnion
{
    quint8 payload[MAX_MODULES * 3];
    struct PayloadSruct
    {
        qint32 lValue;
        qint16 intValue1;
        qint16 intValue2;
        quint8 byteValue1;
        quint8 byteValue2;
        quint8 unused[MAX_MODULES * 3 - sizeof(qint32) - sizeof(qint16) - sizeof(qint16) - 2];
    } payloadStruct;
    DeviceStatusStruct deviceStatus[MAX_MODULES];
};

struct MessageStruct
{
    quint8 startSig[2];
    quint8 messageID;
    qint16 deviceID;
    qint16 controllerID;
    quint8 messageVersion;
    quint8 messageClass;
    PayloadUnion payload;
    quint8 endSig[2];
};

class UDPMessage
{
public:
    UDPMessage(void);
    UDPMessage(const QString &data);
    UDPMessage(const MessageStruct &messageData);

    int getControllerID(void) const { return messageSructure.controllerID; }
    quint8 getDeviceID(void) const { return messageSructure.deviceID; }
    quint8 getMessageID(void) const { return messageSructure.messageID; }
    quint8 getMessageVersion(void) const { return messageSructure.messageVersion; }

    void setControllerID(int value) { messageSructure.controllerID = value; }
    void setDeviceID(int value) { messageSructure.deviceID = value; }
    void setMessageID(quint8 value) { messageSructure.messageID = value; }
    void setMessageVersion(quint8 value) { messageSructure.messageVersion = value; }
    void setMessageClass(quint8 value) { messageSructure.messageClass = value; }
    void setField(quint8 field, quint8 value) { messageSructure.payload.payload[field] = value; }
    void setLValue(long value) { messageSructure.payload.payloadStruct.lValue = value; }
    void setIntValue1(int value) { messageSructure.payload.payloadStruct.intValue1 = value; }
    void setIntValue2(int value) { messageSructure.payload.payloadStruct.intValue2 = value; }
    void setByteValue1(quint8 value) { messageSructure.payload.payloadStruct.byteValue1 = value; }
    void setByteValue2(quint8 value) { messageSructure.payload.payloadStruct.byteValue2 = value; }
    void setDeviceStatus(quint8 index, quint16 deviceID, quint8 value) { messageSructure.payload.deviceStatus[index].id = deviceID;  messageSructure.payload.deviceStatus[index].status = value; }

    quint8 getFieldAsInt(int field) const;
    long getLValue(void) const { return messageSructure.payload.payloadStruct.lValue; }
    int getIntValue1(void) const { return messageSructure.payload.payloadStruct.intValue1; }
    int getIntValue2(void) const { return messageSructure.payload.payloadStruct.intValue2; }
    quint8 getByteValue1(void) const { return messageSructure.payload.payloadStruct.byteValue1; }
    quint8 getByteValue2(void) const { return messageSructure.payload.payloadStruct.byteValue2; }

    const char *getMessageRef(void) const { return (const char *)&messageSructure; }
private:
    MessageStruct messageSructure;
};

#endif // UDPMESSAGE_H
