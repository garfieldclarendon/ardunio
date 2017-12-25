#pragma once

/// Structure used the by the UDPMessage class which contains the actual data of a UDP Message.
struct UDPMessageStruct
{
	unsigned char startSig[2]; ///< 2 byte start-of-message signature set to 0XEE 0XEF.
	unsigned char messageID;  ///< The ID of the message.
	long id; ///<  The ID of either the sender or the target of the message.  Could be the deviceID, controllerID, serialNumber or routeID.
	unsigned char transactionNumber; ///<  The message transaction number.
    unsigned char payload[14];  ///< 14 byte payload accessable as Field[0] to Field[13] through the getField() member.
	unsigned char endSig[2]; ///< 2 byte end-of-message signalture set to 0XEF 0XEE.
};
typedef struct MessageStruct MessageStruct;
/// UDPMessage
/// \Brief Class for handling UDP messages sent between controllers and the application server.
/// This class along with the \ref UDPMessageStruct "Message Structure" contains the data layout of a UDP message.  Messages are sent
/// from controller-to-controller in a peer-to-peer fasion using UDP uni-cast.  Each controller contains a list of "controllers to notify" in its configuration
/// data.  Each time a device needs to send a command or status message, the controller's \ref NetworkManager "Network Manager" sends the message to all controllers
/// contained in the notification list.  Each controller listens for the #SYS_CONTROLLER_ONLINE broadcast message which is sent
/// by each controller upon startup.  Upon resceiving this message, the controller checks to see if the controller's ID is contained in its notification list and, if so, responds
/// by sending device status messages to the sending controller.  If the controller misses this message, the controller sends a #SYS_FIND_CONTROLLER broadcast message
/// every 3 seconds for each controller in its notification list until it recieves a corresponding #SYS_CONTROLLER_ONLINE message from each controller.
class UDPMessage
{
  public:
	  UDPMessage(void);
	UDPMessage(const UDPMessage &other)
	{
		messageStruct = other.messageStruct;
	}
    UDPMessage(const UDPMessageStruct &other)
    {
        messageStruct = other;
    }
	UDPMessage& operator=(const UDPMessage &other) { messageStruct = other.messageStruct; return *this; }

	bool isValid(void) const { return (messageStruct.messageID > 0);  }
	void setMessageID(unsigned char value) { messageStruct.messageID = value;  }
	void setID(long value) { messageStruct.id = value; }
	void setField(unsigned char fieldIndex, unsigned char value) { messageStruct.payload[fieldIndex] = value; }

	char *getRef(void) const { return (char *)&messageStruct;  }

	unsigned char getMessageID(void) const { return messageStruct.messageID; }
	long getID(void) const { return messageStruct.id; }
	unsigned char getTransactionNumber(void) const { return messageStruct.transactionNumber; }
	void setTransactionNumber(unsigned char value) { messageStruct.transactionNumber = value; }
	unsigned char getField(unsigned char fieldIndex) const
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
	static unsigned char nextTransactionNumber;
};
