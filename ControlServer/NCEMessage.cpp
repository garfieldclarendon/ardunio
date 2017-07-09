#include "NCEMessage.h"
#include "NCEInterface.h"

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

    AddressUnion address;
    address.addressInt = number;

    m_messageData.append((char) m_command); // NCE accessory command
    m_messageData.append(address.addressStruct.byteH);    // high address
    m_messageData.append(address.addressStruct.byteL);    // low address
    m_messageData.append(op_1);             // command
    m_messageData.append((char) 0);         // zero out last byte for acc
}

void NCEMessage::accMemoryRead(int address)
{
    m_messageData.clear();
    m_command = READ16_CMD;

    AddressUnion add;
    add.addressInt = address;

    m_messageData.append((char) m_command); // read 16 bytes command
    m_messageData.append(add.addressStruct.byteH);    // high address
    m_messageData.append(add.addressStruct.byteL);    // low address
}

int NCEMessage::getExpectedSize() const
{
    int ret = 1;

    if(m_command == READ16_CMD)
        ret = 16;
    else if(m_command == SW_REV_CMD || m_command == READ_CLOCK_CMD)
        ret = 3;
    else if(m_command == READ_AUI4_CMD)
        ret = 4;
    else if(m_command == READ_AUI2_CMD || m_command == PGM_REG_READ_CMD || m_command == PGM_DIR_READ_CMD)
        ret = 2;
    return ret;
}

