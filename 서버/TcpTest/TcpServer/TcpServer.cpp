#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include "Packet.h"

#define SERVERPORT 9000
#define BUFSIZE    512

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", (LPCWSTR)msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}

struct ClientInfo {
    SOCKET sock;
    SOCKADDR_IN addr;
};

unsigned int WINAPI ClientThread(void* arg) {
    ClientInfo* info = (ClientInfo*)arg;
    SOCKET client_sock = info->sock;
    SOCKADDR_IN client_addr = info->addr;
    delete info;

    printf("[스레드] 클라이언트 처리 시작: %s:%d\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    Packet pkt;
    int retval;

    while (1) {
        retval = recv(client_sock, (char*)&pkt, sizeof(Packet), 0);
        if (retval == SOCKET_ERROR) { err_display("recv()"); break; }
        if (retval == 0) break;

        if (pkt.header != PK_DATA || pkt.endmark != (unsigned short)END_MARK) {
            printf("잘못된 패킷!\n");
            break;
        }

        printf("[%s:%d] %s\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), pkt.data);
    }

    closesocket(client_sock);
    printf("[스레드] 클라이언트 종료: %s:%d\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return 0;
}

int main(int argc, char* argv[])
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    printf("[TCP 서버] 시작. 클라이언트를 기다립니다...\n");

    while (1) {
        // ClientInfo를 힙에 할당 (스레드가 끝날 때 직접 해제)
        ClientInfo* info = new ClientInfo();
        int addrlen = sizeof(info->addr);

        info->sock = accept(listen_sock, (SOCKADDR*)&info->addr, &addrlen);
        if (info->sock == INVALID_SOCKET) {
            err_display("accept()");
            delete info;
            break;
        }

        printf("[메인] 클라이언트 접속: %s:%d\n",
            inet_ntoa(info->addr.sin_addr), ntohs(info->addr.sin_port));

        // 스레드 생성
        HANDLE hThread = (HANDLE)_beginthreadex(
            NULL, 0, ClientThread, info, 0, NULL
        );
        CloseHandle(hThread);  // 핸들만 닫기 (스레드는 계속 실행)
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}
