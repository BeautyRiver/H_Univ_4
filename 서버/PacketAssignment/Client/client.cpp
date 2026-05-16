#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "Packet.h"

#define SERVERIP   "127.0.0.1"
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

int main(int argc, char* argv[])
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    printf("[클라이언트] 서버 연결 성공\n");


    // 로그인
    
    LoginRequestPacket reqPkt;
    ZeroMemory(&reqPkt, sizeof(reqPkt));

    printf("아이디: ");
    scanf_s("%31s", reqPkt.id, (unsigned)sizeof(reqPkt.id));

    printf("비밀번호: ");
    scanf_s("%63s", reqPkt.password, (unsigned)sizeof(reqPkt.password));

    // 헤더 채우기
    reqPkt.header.type = LOGIN_REQUEST;
    reqPkt.header.len = (short)sizeof(LoginRequestPacket);
    reqPkt.header.endmark = END_MARK;

    // 전송
    retval = send(sock, (char*)&reqPkt, sizeof(LoginRequestPacket), 0);
    if (retval == SOCKET_ERROR) {
        err_display("send()");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("[클라이언트] 로그인 요청 전송 완료\n");

    // 로그인 응답 수신
    LoginResponsePacket resPkt;
    ZeroMemory(&resPkt, sizeof(resPkt));

    retval = recv(sock, (char*)&resPkt, sizeof(LoginResponsePacket), 0);
    if (retval == SOCKET_ERROR) { err_display("recv()"); }
    else if (retval == 0) { printf("서버 연결 종료\n"); }
    else {
        if (resPkt.result == LOGIN_SUCCESS) {
            printf("[클라이언트] 로그인 성공!\n");

            // 로비 데이터 수신
            LobbyDataPacket lobbyPkt;
            ZeroMemory(&lobbyPkt, sizeof(lobbyPkt));

            retval = recv(sock, (char*)&lobbyPkt, sizeof(LobbyDataPacket), 0);
            if (retval > 0) {
                printf("\n=== 로비 정보 ===\n");
                printf("닉네임: %s\n", lobbyPkt.nickname);
                printf("골드:   %d\n", lobbyPkt.gold);
                printf("레벨:   %d\n", lobbyPkt.level);
                printf("캐릭터 수: %d\n", lobbyPkt.character_count);
                for (int i = 0; i < lobbyPkt.character_count; i++) {
                    printf("  [%d] %s / 직업ID:%d / 레벨:%d\n",
                        i + 1,
                        lobbyPkt.character_list[i].name,
                        lobbyPkt.character_list[i].class_id,
                        lobbyPkt.character_list[i].level);
                }
            }

            // 로비 대기 루프
            printf("\n[로비] 대기 중... (종료하려면 q 입력)\n");
            char input[10];
            while (1) {
                scanf_s("%9s", input, (unsigned)sizeof(input));
                if (input[0] == 'q') break;
            }
        }
        else {
            printf("[클라이언트] 로그인 실패: ");
            switch (resPkt.fail_reason) {
            case ID_NONE: printf("없는 아이디입니다.\n"); break;
            case PW_ERROR: printf("비밀번호가 틀렸습니다.\n"); break;
            case SERVER_ERROR: printf("서버 오류입니다.\n"); break;
            }
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
