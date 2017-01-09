#pragma once

#include <EtherCard.h>
#include "Message.h"
#include "GlobalDefs.h"

byte Ethernet::buffer[500];
const byte maxSendBuffer = 10;

typedef void (*MessageCallback)(const Message &message);

class UDPNetwork
{
public:
	UDPNetwork(void)
	{
		messageCallback = NULL;
		memset(&messagesToSend, 0, 10);
		lastMessageSentTime = 0;
	}

	void setupNetwork(const byte mac[], MessageCallback callback)
	{
	  Serial.println("setupNetwork");
    
	  messageCallback = callback;
	  if (ether.begin(sizeof(Ethernet::buffer), *mac) == 0)
		Serial.println( "Failed to access Ethernet controller");

	  if (!ether.dhcpSetup())
		Serial.println(F("DHCP failed"));
 
	  ether.printIp("IP:  ", ether.myip);
	  ether.printIp("GW:  ", ether.gwip);
	  ether.printIp("DNS: ", ether.dnsip);
 
	  ether.udpServerListenOnPort(&udpMessageIn, UdpPort);
	}
	
	word processLoop(void)
	{
		word len = ether.packetReceive();
		word pos = ether.packetLoop(len);

		sendMessages();

		return pos;
	}

	void sendMessage(Message *message)
	{
		for(byte x = 0; x < maxSendBuffer; x++)
		{
			if(messagesToSend[x] == 0)
			{
				messagesToSend[x] = message;
				break;
			}
		}
	}

private:
	void sendMessages(void)
	{
		unsigned long t = millis();

		if(t - lastMessageSentTime > sendTimeout)
		{
			lastMessageSentTime = t;
			Message *message;
			for(byte x = 0; x < maxSendBuffer; x++)
			{
				message = messagesToSend[x];
				if(message != 0)
				{
					sendMessage(*message);
					delete messagesToSend[x];
					messagesToSend[x] = 0;
				}
			}
		}
	}

	void sendMessage(const Message &message)
	{
      ether.sendUdp(message.getRef(), sizeof(MessageStruct), UdpPort, UdpBroadcast, UdpPort);
	}

	static void udpMessageIn(uint16_t dest_port, uint8_t ip[4], uint16_t src_port, const char *data, uint16_t len) 
	{
		Serial.println( "udpMessageIn");
		IPAddress src(ip[0], ip[1], ip[2], ip[3]);

		if(len == sizeof(MessageStruct))
		{
			Serial.println( "udpMessageIn size is correct");
			if(messageCallback)
			{
				Serial.println( "udpMessageIn creating message and calling callback");
				Message message(data);
				messageCallback(message);
			}
		}
	}

	static MessageCallback messageCallback;
	Message *messagesToSend[maxSendBuffer];
	unsigned long lastMessageSentTime;
};

MessageCallback UDPNetwork::messageCallback = NULL;
