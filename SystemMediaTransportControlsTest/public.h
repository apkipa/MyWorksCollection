#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#define COBJMACROS

#include <inttypes.h>
#include <stdbool.h>
#include <Windows.h>
#include <roapi.h>
#include <stdio.h>

#include <SystemMediaTransportControlsInterop.h>
#include <windows.media.h>

#pragma comment(lib, "WindowsApp")
#pragma comment(lib, "Winmm")

#define ATOM2LPCSTR(in) ( (LPCSTR)(UINT_PTR)(in) )

static const char g_strAppMainWndClassName[] = "SystemMediaTransportControlsTestMainClass";
static const char g_strAppMainWndName[] = "System Media Transport Controls Test";

static inline LONG_PTR GetWindowUserData(HWND hwnd) {
	return GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

static inline LONG_PTR SetWindowUserData(HWND hwnd, LONG_PTR n) {
	return SetWindowLongPtr(hwnd, GWLP_USERDATA, n);
}

#define PRIGUID "08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%04" PRIX16 "-%04" PRIX16 "%08" PRIX32
#define PRIGUID_Arg(guid) (guid).Data1, (guid).Data2, (guid).Data3, (((guid).Data4[0] << 8) | (guid).Data4[1]),		\
	(((guid).Data4[2] << 8) | (guid).Data4[3]),																		\
	(((DWORD)(guid).Data4[4] << 24) | ((DWORD)(guid).Data4[5] << 16) | ((guid).Data4[6] << 8) | (guid).Data4[7])

#define MIDL_DEFINE_GUID(type, name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8)	\
	EXTERN_C __declspec(selectany) const type name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

MIDL_DEFINE_GUID(IID, IID_ISystemMediaTransportControlsInterop, 0xddb0472d, 0xc911, 0x4a1f, 0x86, 0xd9, 0xdc, 0x3d, 0x71, 0xa9, 0x5f, 0x5a);
MIDL_DEFINE_GUID(IID, IID___x_ABI_CWindows_CMedia_CISystemMediaTransportControls, 0x99fa3ff4, 0x1742, 0x42a6, 0x90, 0x2e, 0x08, 0x7d, 0x41, 0xf9, 0x65, 0xec);
MIDL_DEFINE_GUID(IID, IID___x_ABI_CWindows_CMedia_CISystemMediaTransportControls2, 0xea98d2f6, 0x7f3c, 0x4af2, 0xa5, 0x86, 0x72, 0x88, 0x98, 0x08, 0xef, 0xb1);
MIDL_DEFINE_GUID(IID, IID___FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs, 0x0557e996, 0x7b23, 0x5bae, 0xaa, 0x81, 0xea, 0x0d, 0x67, 0x11, 0x43, 0xa4);