#pragma once

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