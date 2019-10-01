#pragma once

#define _WIN32_WINNT _WIN32_WINNT_WIN10
#define COBJMACROS

#include <inttypes.h>
#include <Windows.h>
#include <ShlObj.h>
#include <stdio.h>

#define ATOM2LPCSTR(in) ( (LPCSTR)(UINT_PTR)(in) )

static const char g_strAppMainWndClassName[] = "VirtualDeskMgrTestMainClass";
static const char g_strAppMainWndName[] = "Virtual Desktop Manager Test";

static inline LONG_PTR GetWindowUserData(HWND hwnd) {
	return GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

static inline LONG_PTR SetWindowUserData(HWND hwnd, LONG_PTR n) {
	return SetWindowLongPtr(hwnd, GWLP_USERDATA, n);
}