#include "UDPMessage.h"
#include "GlobalDefs.h"
#include <string.h>

static quint8 TransactionCounter = 1;

UDPMessage::UDPMessage()
{
    memset(&messageSructure, 0, sizeof(MessageStruct));
    messageSructure.startSig[0] = 0xEE;
    messageSructure.startSig[1] = 0xEF;
    messageSructure.endSig[0] = 0xEF;
    messageSructure.endSig[1] = 0xEE;
    if(TransactionCounter < 255)
        TransactionCounter++;
    else
        TransactionCounter = 1;
    messageSructure.transactionNumber = TransactionCounter;
}

UDPMessage::UDPMessage(const MessageStruct &messageData)
{
    messageSructure = messageData;
}
