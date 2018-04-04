#pragma once

#include <stdint.h>

/// Structure used to store the status of a device.
struct MessageDeviceStruct {
    uint32_t deviceID; ///< ID of the device
    unsigned char status; ///< The device's current status
};
typedef struct MessageDeviceStruct MessageDeviceStruct;

/// Union used the by the UDPMessageStruct which contains an array of 8 device status entries.
union PayloadUnion {
    unsigned char buffer[24];  ///< 24 byte payload accessable as Field[0] to Field[23] through the getField() member.
    MessageDeviceStruct deviceStatus[8]; ///< Array of 8 device status entries
} ;
typedef union PayloadUnion PayloadUnion;

/// Structure used the by the UDPMessage class which contains the actual data of a UDP Message.
struct UDPMessageStruct
{
	unsigned char startSig[2]; ///< 2 byte start-of-message signature set to 0XEE 0XEF.
	unsigned char messageID;  ///< The ID of the message.
	long id; ///<  The ID of either the sender or the target of the message.  Could be the deviceID, controllerID, serialNumber or routeID.
    unsigned char transactionNumber; ///<  The message transaction number.  A unique number auto-generated for each message.
    PayloadUnion payload;  ///< 24 byte payload accessable as Field[0] to Field[23] through the getField() member.
	unsigned char endSig[2]; ///< 2 byte end-of-message signalture set to 0XEF 0XEE.
};
typedef struct UDPMessageStruct UDPMessageStruct;
/// UDPMessage
/// \brief  A class for handling UDP messages sent between controllers and the application server.
///
/// This class along with the \ref UDPMessageStruct "Message Structure" contains the data layout of a UDP message.  Messages are sent
/// from controller-to-controller in a peer-to-peer fasion using UDP uni-cast.  Each controller contains a list of "controllers to notify" in its configuration
/// data.  Each time a device needs to send a command or status message, the controller's \ref NetworkManager "Network Manager" sends the message to all controllers
/// contained in the notification list.  Each controller listens for the #SYS_CONTROLLER_ONLINE broadcast message which is sent
/// by each controller upon startup.  Upon resceiving this message, the controller checks to see if the controller's ID is contained in its notification list and, if so,
/// stores the address of the sending controller which is stored in field[0] - field[3] and then responds by sending device status messages.
/// If the controller misses this message, the controller sends a #SYS_FIND_CONTROLLER broadcast message
/// every 3 seconds for each controller in its notification list until it recieves a corresponding #SYS_CONTROLLER_ONLINE message from each controller.
class UDPMessage
{
  public:
    /// Contstructor
    /// Creates a blank, invalid contsturctor auto-generating the transaction number by incrementing the static
    UDPMessage(void);
    /// Copy constructor
    /// Creates a new UDPMessage copying the information from other creating an exact copy (including the transaction number)
    /// @param other A UDPMessage to use as the soruce
	UDPMessage(const UDPMessage &other)
	{
        m_messageStruct = other.m_messageStruct;
	}
    /// Convenience constructor
    /// Creates a new UDPMessage using the data supplied in the supplied UDPMessageStruct
    /// @param other A UDPMessageStruct to use as the soruce
    UDPMessage(const UDPMessageStruct &other)
    {
        m_messageStruct = other;
    }
    /// Assignment
    /// Replaces the message data; copying the information from other creating an exact copy (including the transaction number)
    /// @param other A UDPMessage to use as the soruce
    UDPMessage& operator=(const UDPMessage &other) { m_messageStruct = other.m_messageStruct; return *this; }

    /// Returns true if the message is considered valid.  At minimum, a valid message has its message ID set to a value greater than zero.
    bool isValid(void) const { return (m_messageStruct.messageID > 0);  }
    /// Setter function.  Sets the message ID.
    /// @param value \ref UDPMessageID "Message ID"
    void setMessageID(unsigned char value) { m_messageStruct.messageID = value;  }
    /// Setter function.  Sets the ID.
    /// @param value an long integer value that represents the controller's ID, serial number or a route ID depending on the \ref UDPMessageID "message".
    void setID(long value) { m_messageStruct.id = value; }
    /// Setter function.  Sets one of the bytes in the 14 byte payload.
    /// @param fieldIndex index of the field to set
    /// @param value a byte containing the new value.
    void setField(unsigned char fieldIndex, unsigned char value) { m_messageStruct.payload.buffer[fieldIndex] = value; }

    /// returns the device ID from the device entry at the given index.
    int getDeviceID(unsigned char index) const { return m_messageStruct.payload.deviceStatus[index].deviceID; }
    /// Setter function.  Sets the device ID of one of the device entries.
    /// @param index index of the field to set
    /// @param value an int containing the device ID.
    void setDeviceID(unsigned char index, int value) { m_messageStruct.payload.deviceStatus[index].deviceID = value; }

    /// returns the status from the device entry at the given index.
    int getDeviceStatus(unsigned char index) const { return m_messageStruct.payload.deviceStatus[index].status; }
    /// Setter function.  Sets the status of one of the device entries.
    /// @param index index of the field to set
    /// @param value an int containing the status of the device.
    void setDeviceStatus(unsigned char index, int value) { m_messageStruct.payload.deviceStatus[index].status = value; }

    /// returns a reference to the internal \ref UDPMessageStruct
    char *getRef(void) const { return (char *)&m_messageStruct;  }

    /// returns the messageID.
    unsigned char getMessageID(void) const { return m_messageStruct.messageID; }
    /// returns the ID
    long getID(void) const { return m_messageStruct.id; }
    /// returns the transaction number
    unsigned char getTransactionNumber(void) const { return m_messageStruct.transactionNumber; }
    /// Setter function.  Sets the transaction number overwriting the auto-generated number;
    /// @param value the new trasaction number.
    void setTransactionNumber(unsigned char value) { m_messageStruct.transactionNumber = value; }
    /// Returns one of the bytes in the 14 byte payload.
    /// @param fieldIndex index of the field to retrieve.
    unsigned char getField(unsigned char fieldIndex) const
	{
        return m_messageStruct.payload.buffer[fieldIndex];
	}
    /// replaces the data of the message creating a copy
    /// Replaces the message data; copying the information from other.  This function does NOT copy the transaction number, rather, a new
    /// transaction number is created.
    /// @param other A UDPMessage to use as the soruce
    void copyFrom(const UDPMessage &other)
	{
        m_messageStruct = other.m_messageStruct;
        if (m_nextTransactionNumber < 255)
            m_nextTransactionNumber++;
		else
            m_nextTransactionNumber++;
        m_messageStruct.transactionNumber = m_nextTransactionNumber;
	}

private:
    UDPMessageStruct m_messageStruct; ///< Message structure which stores the actual message data
    static unsigned char m_nextTransactionNumber; ///< A static byte containing the transaction number.  This value is auto-incremented; rolling over when the value reaches 255
};
