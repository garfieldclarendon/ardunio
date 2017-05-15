#pragma once

#include "Arduino.h"
#include "GlobalDefs.h"

struct MessageStruct
{
	byte startSig[2];
	byte messageID;
	long serialNumber;
	byte version;
	byte payload[14];
	byte endSig[2];
};

typedef struct MessageStruct MessageStruct;

class Message
{
  public:
	  Message(void)
	  {
		  memset(&messageStruct, 0, sizeof(MessageStruct));
		  messageStruct.startSig[0] = 0xEE;
		  messageStruct.startSig[1] = 0xEF;
		  messageStruct.endSig[0] = 0xEF;
		  messageStruct.endSig[1] = 0xEE;
		  messageStruct.version = ControllerVersion;
	  }
	  Message(const char *data)
    {
		memset(&messageStruct, 0, sizeof(MessageStruct));
		memcpy(&messageStruct, data, sizeof(MessageStruct));
    }
	  Message(const Message &other)
	  {
		  messageStruct = other.messageStruct;
	  }
	  Message& operator=(const Message &other) { messageStruct = other.messageStruct; return *this; }

	  bool isValid(void) const { return (messageStruct.messageID > 0);  }
	  void setMessageID(byte messageID) { messageStruct.messageID = messageID;  }
	  void setSerialNumber(long serialNumber) { messageStruct.serialNumber = serialNumber; }
	  void setField(byte fieldIndex, byte value) { messageStruct.payload[fieldIndex] = value; }

	  char *getRef(void) const { return (char *)&messageStruct;  }

	byte getMessageID(void) const { return messageStruct.messageID; }
	long getSerialNumber(void) const { return messageStruct.serialNumber; }
	byte getVersion(void) const { return messageStruct.version; }
    byte getField(byte fieldIndex) const
    {
		return messageStruct.payload[fieldIndex];
    }

private:
	MessageStruct messageStruct;
};

