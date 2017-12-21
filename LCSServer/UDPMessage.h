#ifndef UDPMESSAGE_H
#define UDPMESSAGE_H

#include <QtGlobal>
#include <QString>

#include "GlobalDefs.h"

/// Structure used the UDPMessage class to contain the actual data of a UDP Message.
struct MessageStruct
{
    quint8 startSig[2]; ///< 2 byte start-of-message signature set to 0XEE 0XEF.
    quint8 messageID;  ///< The ID of the message.
    long serialNumber; ///<  The ID of either the sender or the target of the message.  Could be the deviceID, controllerID, serialNumber or routeID.
    quint8 transactionNumber; ///<  The message transaction number.
    quint8 payload[14];  ///< 14 byte payload accessable as Field0 to Field13.
    quint8 endSig[2]; ///< 2 byte end-of-message signalture set to 0XEF 0XEE.
};
/// UDPMessage
/// Brief Class for handling UDP messages sent between controllers and the application server.
/// This class along with the @ref MessageStruct "Message Structure" contains the data layout of a UDP message.  Messages are sent
/// from controller-to-controller in a peer-to-peer fasion using UDP uni-cast.  Each controller contains a list of "controllers to notify" in its configuration
/// data.  Each time a device needs to send a command or status message, the controller's @ref NetworkManager "Network Manager" sends the message to all controllers
/// contained in the notification list.  Each controller listens for the @ref SYS_CONTROLLER_ONLINE "SYS_CONTROLLER_ONLINE" broadcast message which is sent
/// by each controller upon startup.  If the controller misses this message, the controller sends a @ref SYS_FIND_CONTROLLER "SYS_FIND_CONTROLLER" message
/// every 3 seconds.
class UDPMessage
{
public:
    UDPMessage(void);
    UDPMessage(const MessageStruct &messageData);

    long getSerialNumber(void) const { return messageSructure.serialNumber; }
    quint8 getMessageID(void) const { return messageSructure.messageID; }
    quint8 getTransactionNumber(void) const { return messageSructure.transactionNumber; }

    void setSerialNumber(int value) { messageSructure.serialNumber = value; }
    void setMessageID(quint8 value) { messageSructure.messageID = value; }
    void setField(quint8 field, quint8 value) { messageSructure.payload[field] = value; }
    void setTransactionNumber(quint8 value) { messageSructure.transactionNumber = value; }

    quint8 getField(int field) const { return messageSructure.payload[field]; }

    const char *getMessageRef(void) const { return (const char *)&messageSructure; }
private:
    MessageStruct messageSructure;
};

#endif // UDPMESSAGE_H
