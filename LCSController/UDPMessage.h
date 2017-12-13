#pragma once

#include "Arduino.h"

struct UDPMessageStruct
{
	byte startSig[2];
	byte messageID;
	long id;
	byte transactionNumber;
	byte payload[14];
	byte endSig[2];
};

typedef struct MessageStruct MessageStruct;

class UDPMessage
{
  public:
	UDPMessage(void)
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
	UDPMessage(const UDPMessage &other)
	{
		messageStruct = other.messageStruct;
	}
	UDPMessage& operator=(const UDPMessage &other) { messageStruct = other.messageStruct; return *this; }

	bool isValid(void) const { return (messageStruct.messageID > 0);  }
	void setMessageID(byte value) { messageStruct.messageID = value;  }
	void setID(long value) { messageStruct.id = value; }
	void setField(byte fieldIndex, byte value) { messageStruct.payload[fieldIndex] = value; }

	char *getRef(void) const { return (char *)&messageStruct;  }

	byte getMessageID(void) const { return messageStruct.messageID; }
	long getID(void) const { return messageStruct.id; }
	byte getTransactionNumber(void) const { return messageStruct.transactionNumber; }
	void setTransactionNumber(byte value) { messageStruct.transactionNumber = value; }
	byte getField(byte fieldIndex) const
	{
		return messageStruct.payload[fieldIndex];
	}
	void copyFrom(const UDPMessage &other)
	{
		messageStruct = other.messageStruct;
		if (nextTransactionNumber < 255)
			nextTransactionNumber++;
		else
			nextTransactionNumber++;
		messageStruct.transactionNumber = nextTransactionNumber;
	}

private:
	UDPMessageStruct messageStruct;
	static byte nextTransactionNumber;
};
