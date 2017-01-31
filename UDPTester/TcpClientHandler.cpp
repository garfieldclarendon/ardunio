#include <QTcpSocket>

#include "TcpClientHandler.h"
#include "Database.h"
#include "UDPMessage.h"

TcpClientHandler::TcpClientHandler(QTcpSocket *clientSocket) : QObject(clientSocket), socket(clientSocket)
{
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void TcpClientHandler::readyRead()
{
    qDebug("readyRead");
    if(socket->bytesAvailable() >= sizeof(MessageStruct))
    {
        MessageStruct messageStruct;
        socket->read((char *)&messageStruct, sizeof(MessageStruct));

        if(messageStruct.startSig[0] == 0xEE && messageStruct.endSig[1] == 0xEE)
        {

        }
        else
        {
            emit logError(2, 1, "Error reading tcp message.  Invalid signature.");
        }
    }
}


