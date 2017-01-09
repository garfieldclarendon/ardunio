#include "UDPMessage.h"
#include "GlobalDefs.h"

UDPMessage::UDPMessage()
{
    memset(&messageSructure, 0, sizeof(MessageStruct));
    messageSructure.startSig = 255;
    messageSructure.endSig = 255;
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
