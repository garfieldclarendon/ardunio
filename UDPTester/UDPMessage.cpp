#include "UDPMessage.h"
#include "GlobalDefs.h"

UDPMessage::UDPMessage()
{
    memset(&messageSructure, 0, sizeof(MessageStruct));
    messageSructure.startSig[0] = 0xEE;
    messageSructure.startSig[1] = 0xEF;
    messageSructure.endSig[0] = 0xEF;
    messageSructure.endSig[1] = 0xEE;
    messageSructure.messageVersion = ControllerVersion;
}

UDPMessage::UDPMessage(const QString & /*data*/)
{
    memset(&messageSructure, 0, sizeof(MessageStruct));
}

UDPMessage::UDPMessage(const MessageStruct &messageData)
{
    messageSructure = messageData;
}

quint8 UDPMessage::getFieldAsInt(int field) const
{
    return messageSructure.payload.payload[field];
}
