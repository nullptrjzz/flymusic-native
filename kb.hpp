#pragma once
#define _WIN32_WINNT  0400
#define STRICT
#define WIN32_LEAN_AND_MEAN

#include "push.hpp"
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <WinUser.h>
#include <thread>

#define logd printf

unsigned int tid = 0;
HHOOK kbHook = 0;
int rpcPort = 0;

BOOL CALLBACK conHandler(DWORD) {
	PostThreadMessage(tid, WM_QUIT, 0, 0);
	return TRUE;
}

LRESULT CALLBACK kbProc(int code, WPARAM w, LPARAM l) {
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)l;
	const char* data = NULL;
	if (w == WM_KEYDOWN) {
		switch (p->vkCode) {
			case VK_MEDIA_NEXT_TRACK:
				data = "KEY_HOOK next";
				break;
			case VK_MEDIA_PREV_TRACK:
				data = "KEY_HOOK prev";
				break;
			case VK_MEDIA_STOP:
				data = "KEY_HOOK stop";
				break;
			case VK_MEDIA_PLAY_PAUSE:
				data = "KEY_HOOK play";
				break;
		}
	}
	logd("%d - vkCode [%04x], scanCode [%04x], data: %s\n", w, p->vkCode, p->scanCode, data);
	if (data) {
		push(rpcPort, data, strlen(data));
	}
	return CallNextHookEx(kbHook, code, w, l);
}

void backgroundTask(int port) {
	rpcPort = port;
	tid = GetCurrentThreadId();
	SetConsoleCtrlHandler(&conHandler, TRUE);
	kbHook = SetWindowsHookEx(WH_KEYBOARD_LL, &kbProc, GetModuleHandle(L"flyaudio"), 0);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(kbHook);
}

void initKbHook(int port) {
	std::thread t(backgroundTask, port);
	t.join();
}