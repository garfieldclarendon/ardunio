#include "UDPMessage.h"
#include "GlobalDefs.h"
#include <string.h>

UDPMessage::UDPMessage()
{
    memset(&messageSructure, 0, sizeof(MessageStruct));
    messageSructure.startSig[0] = 0xEE;
    messageSructure.startSig[1] = 0xEF;
    messageSructure.endSig[0] = 0xEF;
    messageSructure.endSig[1] = 0xEE;
    messageSructure.version = ControllerVersion;
}

UDPMessage::UDPMessage(const MessageStruct &messageData)
{
    messageSructure = messageData;
}
