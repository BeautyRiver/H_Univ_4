#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include "Packet.h"

#define SERVERPORT 9000

void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// 더미 User DB 
struct UserDB {
    char id[32];
    char password[64];
    char nickname[32];
    int  gold;
    short level;
};

UserDB db[] = {
    {"admin", "1234", "관리자", 9999, 99},
    {"user1", "abcd", "고수", 1000, 10},
    {"kmk", "kmk", "민강", 10000000, 200}
};
int db_count = 3;

// 캐릭터 DB
struct CharacterDB {
    char owner_id[32];  // 어떤 유저 소유인지
    Character data;     // 캐릭터 데이터
};

CharacterDB charDB[] = {
    {"admin", {"검사", 0, 12}},
    {"admin", {"썬콜", 1, 145}},
    {"user1", {"페이지", 0, 78}},
    {"user1", {"불독", 1, 64}},
    {"user1", {"클래릭", 1, 35}},
    {"kmk", {"시프", 2, 56}},
    {"kmk", {"바이퍼", 3, 137}},
    {"kmk", {"보우마스터", 4, 198}},
};
int charDB_count = 8;

// 로그인 검증
char CheckLogin(const char* id, const char* password, int* out_index)
{
    for (int i = 0; i < db_count; i++) {
        if (strcmp(db[i].id, id) == 0) {
            if (strcmp(db[i].password, password) == 0) {
                *out_index = i;
                return LOGIN_SUCCESS;
            }
            else {
                return LOGIN_FAIL;  // 아이디는 있는데 비번 틀림
            }
        }
    }
    return LOGIN_FAIL;  // 아이디 없음
}

struct ClientInfo {
    SOCKET sock;
    SOCKADDR_IN addr;
};

unsigned int WINAPI ClientThread(void* arg)
{
    ClientInfo* info = (ClientInfo*)arg;
    SOCKET client_sock = info->sock;
    SOCKADDR_IN clientaddr = info->addr;
    delete info;

    printf("[스레드] 클라이언트 접속: %s:%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    int retval;

    // LoginRequestPacket 수신
    LoginRequestPacket reqPkt;
    ZeroMemory(&reqPkt, sizeof(reqPkt));

    retval = recv(client_sock, (char*)&reqPkt, sizeof(LoginRequestPacket), 0);
    if (retval <= 0) {
        closesocket(client_sock);
        return 0;
    }

    // 헤더 검증
    if (reqPkt.header.type != LOGIN_REQUEST ||
        reqPkt.header.endmark != (short)END_MARK) {
        printf("[서버] 잘못된 패킷\n");
        closesocket(client_sock);
        return 0;
    }

    printf("[서버] 로그인 요청 - ID: %s\n", reqPkt.id);

    // 로그인 검증
    int db_index = -1;
    char login_result = CheckLogin(reqPkt.id, reqPkt.password, &db_index);

    // LoginResponsePacket 전송
    LoginResponsePacket resPkt;
    ZeroMemory(&resPkt, sizeof(resPkt));
    resPkt.header.type = LOGIN_RESPONSE;
    resPkt.header.len = (short)sizeof(LoginResponsePacket);
    resPkt.header.endmark = (short)END_MARK;
    resPkt.result = login_result;

    if (login_result == LOGIN_FAIL) {
        // 아이디 존재 여부로 실패 원인 구분
        bool id_exists = false;
        for (int i = 0; i < db_count; i++) {
            if (strcmp(db[i].id, reqPkt.id) == 0) {
                id_exists = true;
                break;
            }
        }
        resPkt.fail_reason = id_exists ? PW_ERROR : ID_NONE;
        printf("[서버] 로그인 실패 - %s\n", id_exists ? "비밀번호 오류" : "없는 아이디");
    }

    retval = send(client_sock, (char*)&resPkt, sizeof(LoginResponsePacket), 0);
    if (retval == SOCKET_ERROR) {
        err_display("send()");
        closesocket(client_sock);
        return 0;
    }

    // 실패면 여기서 종료
    if (login_result == LOGIN_FAIL) {
        closesocket(client_sock);
        return 0;
    }

    printf("[서버] 로그인 성공 - %s\n", db[db_index].nickname);

    // LobbyDataPacket 전송
    LobbyDataPacket lobbyPkt;
    ZeroMemory(&lobbyPkt, sizeof(lobbyPkt));
    lobbyPkt.header.type = LOBBY_DATA;
    lobbyPkt.header.len = (short)sizeof(LobbyDataPacket);
    lobbyPkt.header.endmark = (short)END_MARK;

    // 유저 정보 채우기
    strncpy_s(lobbyPkt.nickname, db[db_index].nickname, sizeof(lobbyPkt.nickname) - 1);
    lobbyPkt.gold = db[db_index].gold;
    lobbyPkt.level = db[db_index].level;
    
    // 해당 유저의 캐릭터만 가져오기
    lobbyPkt.character_count = 0;
    for (int i = 0; i < charDB_count; i++) {
        if (strcmp(charDB[i].owner_id, reqPkt.id) == 0) {
            lobbyPkt.character_list[lobbyPkt.character_count] = charDB[i].data;
            lobbyPkt.character_count++;
            if (lobbyPkt.character_count >= MAX_CHARACTER) break;
        }
    }

    retval = send(client_sock, (char*)&lobbyPkt, sizeof(LobbyDataPacket), 0);
    if (retval == SOCKET_ERROR) {
        err_display("send()");
    }

    closesocket(client_sock);
    printf("[스레드] 클라이언트 종료: %s:%d\n",
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    return 0;
}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);

    if (bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
        err_quit("bind()");
    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR)
        err_quit("listen()");

    printf("[서버] 시작. 클라이언트를 기다립니다...\n");

    while (1) {
        ClientInfo* info = new ClientInfo();
        int addrlen = sizeof(info->addr);

        info->sock = accept(listen_sock, (SOCKADDR*)&info->addr, &addrlen);
        if (info->sock == INVALID_SOCKET) {
            err_display("accept()");
            delete info;
            break;
        }

        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ClientThread, info, 0, NULL);
        CloseHandle(hThread);
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}