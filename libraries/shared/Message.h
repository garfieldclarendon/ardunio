#pragma once

#include "Arduino.h"
#include "GlobalDefs.h"

struct DeviceStatusStruct
{
	short id;
	byte status;
};

union PayloadUnion
{
	byte payload[MAX_MODULES * 3];
	struct PayloadStruct
	{
		long lValue;
		short intValue1;
		short intValue2;
		byte byteValue1;
		byte byteValue2;
		byte unused[MAX_MODULES * 3 - sizeof(long) - sizeof(short) - sizeof(short) - 2];
	} payloadStruct;
	DeviceStatusStruct deviceStatus[MAX_MODULES];
};
struct MessageStruct
{
	byte startSig[2];
	byte messageID;
	short  deviceID;
	short controllerID;
	byte messageVersion;
	byte messageClass;
	PayloadUnion payload;
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
		  messageStruct.messageVersion = ControllerVersion;
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
	  void setControllerID(short controllerID) { messageStruct.controllerID = controllerID;  }
	  void setDeviceID(short deviceID) { messageStruct.deviceID = deviceID; }
//	  void setMessageVersion(byte messageVersion) { messageStruct.messageVersion = messageVersion; }
	  void setMessageClass(byte messageClass) { messageStruct.messageClass = messageClass;  }
	  void setField(byte fieldIndex, byte value) { messageStruct.payload.payload[fieldIndex] = value; }
	  void setLValue(long value) { messageStruct.payload.payloadStruct.lValue = value; }
	  void setIntValue1(short value) { messageStruct.payload.payloadStruct.intValue1 = value; }
	  void setIntValue2(short value) { messageStruct.payload.payloadStruct.intValue2 = value; }
	  void setByteValue1(byte value) { messageStruct.payload.payloadStruct.byteValue1 = value; }
	  void setByteValue2(byte value) { messageStruct.payload.payloadStruct.byteValue2 = value; }
	  void setDeviceStatus(byte index, short deviceID, byte value) { messageStruct.payload.deviceStatus[index].id = deviceID;  messageStruct.payload.deviceStatus[index].status = value; }

	  char *getRef(void) const { return (char *)&messageStruct;  }

	byte getMessageID(void) const { return messageStruct.messageID; }
	short getControllerID(void) const { return messageStruct.controllerID; }
	short  getDeviceID(void) const { return messageStruct.deviceID; }
	byte getMessageVersion(void) const { return messageStruct.messageVersion; }
	byte getMessageClass(void) const { return messageStruct.messageClass;  }
    byte getField(byte fieldIndex) const
    {
		return messageStruct.payload.payload[fieldIndex];
    }
	long getLValue(void) const { return messageStruct.payload.payloadStruct.lValue; }
	short getIntValue1(void) const { return messageStruct.payload.payloadStruct.intValue1; }
	short getIntValue2(void) const { return messageStruct.payload.payloadStruct.intValue2; }
	byte getByteValue1(void) const { return messageStruct.payload.payloadStruct.byteValue1; }
	byte getByteValue2(void) const { return messageStruct.payload.payloadStruct.byteValue2; }
	byte getDeviceStatus(byte index) const { return messageStruct.payload.deviceStatus[index].status; }
	byte getDeviceStatusID(byte index) const { return messageStruct.payload.deviceStatus[index].id; }

private:
	MessageStruct messageStruct;
};

