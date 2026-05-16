#pragma once
#pragma pack(push, 1)

#define MAX_CHARACTER 5

#define END_MARK 0xFFFF
#define LOGIN_REQUEST  0x01
#define LOGIN_RESPONSE 0x02
#define LOBBY_DATA     0x03

#define LOGIN_FAIL 0
#define LOGIN_SUCCESS 1

#define ID_NONE 0
#define PW_ERROR 1
#define SERVER_ERROR 2

struct PacketHeader {
    short  len;      // 패킷 전체 크기
    short  type;     // 패킷 종류 
    short  endmark;  
};

struct Character {
    char name[32];   // 캐릭터 이름
    short class_id;  // 직업
    short level;     // 캐릭터 레벨
};

struct LoginRequestPacket {
    PacketHeader header;
    char id[32];      
    char password[64];
};

struct LoginResponsePacket {
    PacketHeader header;
    char  result;           // 0 = 실패, 1 = 성공
    char  fail_reason;      // 0 = 없는 아이디, 1 = 비밀번호 틀림, 2 = 서버 오류
};

struct LobbyDataPacket {
    PacketHeader header;     
    char  nickname[32];
    int   gold;
    short level;
    char  character_count;    
    Character character_list[MAX_CHARACTER];
};

#pragma pack(pop)

