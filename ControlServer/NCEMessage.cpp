#include "NCEMessage.h"

NCEMessage::NCEMessage(void)
    : m_command(-1)
{

}

void NCEMessage::accDecoder(int number, bool closed)
{
    m_messageData.clear();
    m_command = ACC_CMD;
    char op_1;
    if (closed)
    {
        op_1 = 0x03;
    } else
    {
        op_1 = 0x04;
    }

    int addr_h = number / 256;
    int addr_l = number & 0xFF;

    m_messageData.append((char) m_command); // NCE accessory command
    m_messageData.append((char) addr_h);    // high address
    m_messageData.append((char) addr_l);    // low address
    m_messageData.append(op_1);             // command
    m_messageData.append((char) 0);         // zero out last byte for acc
}

void NCEMessage::accMemoryRead(int address)
{
    m_messageData.clear();
    m_command = READ16_CMD;
    int addr_h = address / 256;
    int addr_l = address & 0xFF;

    m_messageData.append((char) m_command); // read 16 bytes command
    m_messageData.append((char) addr_h);    // high address
    m_messageData.append((char) addr_l);    // low address
}

