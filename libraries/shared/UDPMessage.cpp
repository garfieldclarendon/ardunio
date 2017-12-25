#include "UDPMessage.h"
#include <string.h>

unsigned char UDPMessage::nextTransactionNumber = 1;

UDPMessage::UDPMessage(void)
{
	memset(&messageStruct, 0, sizeof(UDPMessageStruct));
	messageStruct.startSig[0] = 0xEE;
	messageStruct.startSig[1] = 0xEF;
	messageStruct.endSig[0] = 0xEF;
	messageStruct.endSig[1] = 0xEE;
	if (nextTransactionNumber < 255)
		nextTransactionNumber++;
	else
		nextTransactionNumber++;
	messageStruct.transactionNumber = nextTransactionNumber;
}
