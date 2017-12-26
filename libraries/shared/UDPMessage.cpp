#include "UDPMessage.h"
#include <string.h>

unsigned char UDPMessage::m_nextTransactionNumber = 1;

UDPMessage::UDPMessage(void)
{
    memset(&m_messageStruct, 0, sizeof(UDPMessageStruct));
    m_messageStruct.startSig[0] = 0xEE;
    m_messageStruct.startSig[1] = 0xEF;
    m_messageStruct.endSig[0] = 0xEF;
    m_messageStruct.endSig[1] = 0xEE;
    if (m_nextTransactionNumber < 255)
        m_nextTransactionNumber++;
	else
        m_nextTransactionNumber++;
    m_messageStruct.transactionNumber = m_nextTransactionNumber;
}
