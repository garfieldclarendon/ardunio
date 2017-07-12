#ifndef NCEMESSAGE_H
#define NCEMESSAGE_H

#include <QVector>

class NCEMessage
{
public:
    const quint8 NOOP_CMD = 0x80;            // NCE No Op Command, NCE-USB yes
    const quint8 ASSIGN_CAB_CMD = 0x81;      // NCE Assign loco to cab command, NCE-USB no
    const quint8 READ_CLOCK_CMD = 0x82;      // NCE read clock command, NCE-USB no
    const quint8 STOP_CLOCK_CMD = 0x83;      // NCE stop clock command, NCE-USB no
    const quint8 START_CLOCK_CMD = 0x84;     // NCE start clock command, NCE-USB no
    const quint8 SET_CLOCK_CMD = 0x85;       // NCE set clock command, NCE-USB no
    const quint8 CLOCK_1224_CMD = 0x86;      // NCE change clock 12/24 command, NCE-USB no
    const quint8 CLOCK_RATIO_CMD = 0x87;     // NCE set clock ratio command, NCE-USB no
    const quint8 DEQUEUE_CMD = 0x88;         // NCE dequeue packets based on loco addr, NCE-USB no
    const quint8 ENABLE_TRACK_CMD = 0x89;    // NCE enable track/kill programm track, NCE-USB no
    const quint8 READ_AUI4_CMD = 0x8A;       // NCE read status of AUI yy, returns four bytes, NCE-USB no
    const quint8 DISABLE_TRACK_CMD = 0x89;   // NCE enable program/kill main track, NCE-USB no
    const quint8 DUMMY_CMD = 0x8C;           // NCE Dummy instruction, NCE-USB yes
    const quint8 SPEED_MODE_CMD = 0x8D;      // NCE set speed mode, NCE-USB no
    const quint8 WRITE_N_CMD = 0x8E;         // NCE write up to 16 bytes of memory command, NCE-USB no
    const quint8 READ16_CMD = 0x8F;          // NCE read 16 bytes of memory command, NCE-USB no
    const quint8 DISPLAY3_CMD = 0x90;        // NCE write 16 char to cab display line 3, NCE-USB no
    const quint8 DISPLAY4_CMD = 0x91;        // NCE write 16 char to cab display line 4, NCE-USB no
    const quint8 DISPLAY2_CMD = 0x92;        // NCE write 8 char to cab display line 2 right, NCE-USB no
    const quint8 QUEUE3_TMP_CMD = 0x93;      // NCE queue 3 bytes to temp queue, NCE-USB no
    const quint8 QUEUE4_TMP_CMD = 0x94;      // NCE queue 4 bytes to temp queue, NCE-USB no
    const quint8 QUEUE5_TMP_CMD = 0x95;      // NCE queue 5 bytes to temp queue, NCE-USB no
    const quint8 QUEUE6_TMP_CMD = 0x96;      // NCE queue 6 bytes to temp queue, NCE-USB no
    const quint8 WRITE1_CMD = 0x97;          // NCE write 1 bytes of memory command, NCE-USB no
    const quint8 WRITE2_CMD = 0x98;          // NCE write 2 bytes of memory command, NCE-USB no
    const quint8 WRITE4_CMD = 0x99;          // NCE write 4 bytes of memory command, NCE-USB no
    const quint8 WRITE8_CMD = 0x9A;          // NCE write 8 bytes of memory command, NCE-USB no
    const quint8 READ_AUI2_CMD = 0x9B;       // NCE read status of AUI yy, returns two bytes, NCE-USB >= 1.65
    const quint8 MACRO_CMD = 0x9C;           // NCE execute macro n, NCE-USB yes
    const quint8 READ1_CMD = 0x9D;           // NCE read 1 byte of memory command, NCE-USB no
    const quint8 PGM_TRK_ON_CMD = 0x9E;      // NCE enter program track  command, NCE-USB yes
    const quint8 PGM_TRK_OFF_CMD = 0x9F;     // NCE exit program track  command, NCE-USB yes
    const quint8 PGM_PAGE_WRITE_CMD = 0xA0;  // NCE program track, page mode write command, NCE-USB yes
    const quint8 PGM_PAGE_READ_CMD = 0xA1;   // NCE program track, page mode read command, NCE-USB yes
    const quint8 LOCO_CMD = 0xA2;            // NCE loco control command, NCE-USB yes
    const quint8 QUEUE3_TRK_CMD = 0xA3;      // NCE queue 3 bytes to track queue, NCE-USB no
    const quint8 QUEUE4_TRK_CMD = 0xA4;      // NCE queue 4 bytes to track queue, NCE-USB no
    const quint8 QUEUE5_TRK_CMD = 0xA5;      // NCE queue 5 bytes to track queue, NCE-USB no
    const quint8 PGM_REG_WRITE_CMD = 0xA6;   // NCE program track, register mode write command, NCE-USB yes
    const quint8 PGM_REG_READ_CMD = 0xA7;    // NCE program track, register mode read command, NCE-USB yes
    const quint8 PGM_DIR_WRITE_CMD = 0xA8;   // NCE program track, direct mode write command, NCE-USB yes
    const quint8 PGM_DIR_READ_CMD = 0xA9;    // NCE program track, direct mode read command, NCE-USB yes
    const quint8 SW_REV_CMD = 0xAA;          // NCE get EPROM revision cmd, Reply Format: VV.MM.mm, NCE-USB yes
    const quint8 RESET_SOFT_CMD = 0xAB;      // NCE soft reset command, NCE-USB no
    const quint8 RESET_HARD_CMD = 0xAC;      // NCE hard reset command, NCE-USB no
    const quint8 ACC_CMD = 0xAD;             // NCE accessory command, NCE-USB yes
    const quint8 OPS_PROG_LOCO_CMD = 0xAE;   // NCE ops mode program loco, NCE-USB yes
    const quint8 OPS_PROG_ACCY_CMD = 0xAF;   // NCE ops mode program accessories, NCE-USB yes
    const quint8 FACTORY_TEST_CMD = 0xB0;    // NCE factory test, NCE-USB yes
    const quint8 USB_SET_CAB_CMD = 0xB1;     // NCE set cab address in USB, NCE-USB yes
    const quint8 USB_MEM_POINTER_CMD = 0xB3; // NCE set memory context poconst inter, NCE-USB >= 1.65
    const quint8 USB_MEM_WRITE_CMD = 0xB4;   // NCE write memory, NCE-USB >= 1.65
    const quint8 USB_MEM_READ_CMD = 0xB5;    // NCE read memory, NCE-USB >= 1.65

    // NCE Command 0xA2 sends speed or function packets to a locomotive
    // 0xA2 sub commands speed and functions
    const quint8 LOCO_CMD_REV_28SPEED = 0x01;  // set loco speed 28 steps reverse
    const quint8 LOCO_CMD_FWD_28SPEED = 0x02;  // set loco speed 28 steps forward
    const quint8 LOCO_CMD_REV_128SPEED = 0x03; // set loco speed 128 steps reverse
    const quint8 LOCO_CMD_FWD_128SPEED = 0x04; // set loco speed 128 steps forward
    const quint8 LOCO_CMD_REV_ESTOP = 0x05;    // emergency stop reverse
    const quint8 LOCO_CMD_FWD_ESTOP = 0x06;    // emergency stop forward
    const quint8 LOCO_CMD_FG1 = 0x07;          // function group 1
    const quint8 LOCO_CMD_FG2 = 0x08;          // function group 2
    const quint8 LOCO_CMD_FG3 = 0x09;          // function group 3
    const quint8 LOCO_CMD_FG4 = 0x15;          // function group 4
    const quint8 LOCO_CMD_FG5 = 0x16;          // function group 5

    // OxA2 sub commands consist
    const quint8 LOCO_CMD_REV_CONSIST_LEAD = 0x0A;    // reverse consist address for lead loco
    const quint8 LOCO_CMD_FWD_CONSIST_LEAD = 0x0B;    // forward consist address for lead loco
    const quint8 LOCO_CMD_REV_CONSIST_REAR = 0x0C;    // reverse consist address for rear loco
    const quint8 LOCO_CMD_FWD_CONSIST_REAR = 0x0D;    // forward consist address for rear loco
    const quint8 LOCO_CMD_REV_CONSIST_MID = 0x0E;     // reverse consist address for additional loco
    const quint8 LOCO_CMD_FWD_CONSIST_MID = 0x0F;     // forward consist address for additional loco
    const quint8 LOCO_CMD_DELETE_LOCO_CONSIST = 0x10; // Delete loco from consist
    const quint8 LOCO_CMD_KILL_CONSIST = 0x11;        // Kill consist

    NCEMessage(void);
    NCEMessage(int command) : m_command(command) { }
    NCEMessage(const NCEMessage &other) { copy(other); }

    bool isValid(void) const { return m_messageData.size() > 0; }
    NCEMessage& operator = (const NCEMessage &other)
    {
        copy(other);
        return *this;
    }

    int getCommand(void) const { return m_command; }
    QVector<quint8> getMessageData(void) const { return m_messageData; }
    QVector<quint8> getResultData(void) const { return m_resultData; }
    void setResultData(const QVector<quint8> &value) { m_resultData = value; }

    void accDecoder(int number, bool normal);
    void accMemoryRead(int address);
    void getVersion(void);

    int getExpectedSize(void) const;

private:
    void copy(const NCEMessage &other)
    {
        m_command = other.m_command;
        m_messageData = other.m_messageData;
        m_resultData = other.m_resultData;
    }

    quint8 m_command;
    QVector<quint8> m_messageData;
    QVector<quint8> m_resultData;
};

#endif // NCEMESSAGE_H
