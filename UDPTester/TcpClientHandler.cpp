#include <QTcpSocket>

#include "TcpClientHandler.h"

TcpClientHandler::TcpClientHandler(QTcpSocket *clientSocket)
    : QObject(clientSocket), socket(clientSocket)
{
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void TcpClientHandler::readyRead()
{
    qDebug("readyRead");
}


