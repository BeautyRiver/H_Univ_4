#pragma once

#pragma pack(push, 1)

#define PK_DATA  0x01
#define END_MARK 0xFFFF

struct Packet {
    short len;
    char  header;
    char  data[20];
    unsigned short endmark;
};

#pragma pack(pop)