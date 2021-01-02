#pragma once

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma comment(lib, "WS2_32.lib")


int push(int port, const char* content, int len) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err) return err;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock == INVALID_SOCKET) {
		WSACleanup();
		return -1;
	}

	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	int result = sendto(sock, content, len, 0, (SOCKADDR*)&addr, sizeof(addr));
	closesocket(sock);
	WSACleanup();
	return 0;
}

#else
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int push(int port, const char* content, int len) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    sendto(sock_fd, content, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sock_fd);
    return 0;
}
#endif