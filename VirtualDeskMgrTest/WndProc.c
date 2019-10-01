#include "public.h"

#define TIMER_ID_UpdateData 10001

int GetTextHeight(HDC hdc) {
	TEXTMETRICA tm;
	GetTextMetricsA(hdc, &tm);
	return tm.tmHeight;
}

LRESULT WndProc_NonClientCreate(HWND hwnd, LPCREATESTRUCTA lpCreateStruct) {
	IVirtualDesktopManager *piVdm;
	if (CoCreateInstance(&CLSID_VirtualDesktopManager, NULL, CLSCTX_ALL, &IID_IVirtualDesktopManager, (void**)&piVdm) != S_OK) {
		MessageBoxA(NULL, "Sorry, your system does not support virtual desktop mamager!", "Error", MB_ICONERROR);
		return FALSE;
	}
	SetWindowUserData(hwnd, (LONG_PTR)piVdm);
	return TRUE;
}

LRESULT WndProc_Create(HWND hwnd, LPCREATESTRUCTA lpCreateStruct) {
	SetTimer(hwnd, TIMER_ID_UpdateData, 1000, NULL);
	return 0;
}

void WndProc_Timer_UpdateData(HWND hwnd) {
	InvalidateRect(hwnd, NULL, TRUE);
}

void WndProc_Timer(HWND hwnd, UINT_PTR nTimerId) {
	switch (nTimerId) {
	case TIMER_ID_UpdateData:
		WndProc_Timer_UpdateData(hwnd);
		break;
	}
}

#define PRIGUID "08" PRIX32 "-%04" PRIX16 "-%04" PRIX16 "-%04" PRIX16 "-%04" PRIX16 "%08" PRIX32
#define PRIGUID_Arg(guid) (guid).Data1, (guid).Data2, (guid).Data3, (((guid).Data4[0] << 8) | (guid).Data4[1]),		\
	(((guid).Data4[2] << 8) | (guid).Data4[3]),																		\
	(((DWORD)(guid).Data4[4] << 24) | ((DWORD)(guid).Data4[5] << 16) | ((guid).Data4[6] << 8) | (guid).Data4[7])

void WndProc_Paint(HWND hwnd) {
	IVirtualDesktopManager *piVdm;
	char strBuf[4096];
	int nTextHeight;
	PAINTSTRUCT ps;
	RECT rtClient;
	int nOffset;
	HWND hwCur;
	GUID guid;
	BOOL bRet;

	piVdm = (IVirtualDesktopManager*)GetWindowUserData(hwnd);
	GetClientRect(hwnd, &rtClient);

	BeginPaint(hwnd, &ps);
	nTextHeight = GetTextHeight(ps.hdc);

	if (IVirtualDesktopManager_GetWindowDesktopId(piVdm, hwnd, &guid) != S_OK)
		guid = (GUID) { 0 };
	nOffset = sprintf(strBuf, "Current virtual desktop id: %" PRIGUID "\n", PRIGUID_Arg(guid));

	hwCur = NULL;
	while ((hwCur = FindWindowExA(NULL, hwCur, g_strAppMainWndClassName, g_strAppMainWndName)) != NULL) {
		if (hwCur == hwnd)
			continue;
		if (IVirtualDesktopManager_IsWindowOnCurrentVirtualDesktop(piVdm, hwCur, &bRet) == S_OK) {
			if (bRet) {
				nOffset += sprintf(strBuf + nOffset, "Found another window on current desktop\n");
			}
			else {
				if (IVirtualDesktopManager_GetWindowDesktopId(piVdm, hwCur, &guid) != S_OK)
					guid = (GUID) { 0 };
				nOffset += sprintf(strBuf + nOffset, "Found another window on desktop %" PRIGUID "\n", PRIGUID_Arg(guid));
			}
		}
	}

	nOffset += sprintf(
		strBuf + nOffset,
		"Type corresponding number to take current window to the desktop which the other window is on."
	);

	DrawTextA(ps.hdc, strBuf, -1, &rtClient, 0);

	EndPaint(hwnd, &ps);
}

void WndProc_KeyDown(
	HWND hwnd,
	BYTE byteVirtualKeyCode,
	BYTE byteRepeatCount,
	BYTE byteScanCode,
	BOOL bIsExtended,
	BOOL bContextCode,
	BOOL bPreviousKeyState,
	BOOL bTransitionState
) {
	IVirtualDesktopManager *piVdm;
	int nCycleTimes;
	HWND hwCur;
	GUID guid;
	int nCnt;

	piVdm = (IVirtualDesktopManager*)GetWindowUserData(hwnd);

	switch (byteVirtualKeyCode - '0') {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		nCycleTimes = byteVirtualKeyCode - '0';
		break;
	default:
		return;
	}

	nCnt = 0;
	hwCur = NULL;
	while ((hwCur = FindWindowExA(NULL, hwCur, g_strAppMainWndClassName, g_strAppMainWndName)) != NULL) {
		if (hwCur == hwnd)
			continue;
		if (nCnt == nCycleTimes) {
			if (
				IVirtualDesktopManager_GetWindowDesktopId(piVdm, hwCur, &guid) != S_OK ||
				IVirtualDesktopManager_MoveWindowToDesktop(piVdm, hwnd, &guid) != S_OK
				) {
				MessageBoxA(hwnd, "Cannot take current window to another desktop.", "Error", MB_ICONERROR);
			}
		}
		nCnt++;
	}
}

void WndProc_Destroy(HWND hwnd) {
	IVirtualDesktopManager *piVdm;

	KillTimer(hwnd, TIMER_ID_UpdateData);

	piVdm = (IVirtualDesktopManager*)GetWindowUserData(hwnd);
	IVirtualDesktopManager_Release(piVdm);

	PostQuitMessage(EXIT_SUCCESS);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT nRet;

	switch (msg) {
	case WM_NCCREATE:
		nRet = WndProc_NonClientCreate(hwnd, (LPCREATESTRUCTA)lParam);
		if (nRet == FALSE)
			return FALSE;
		break;
	case WM_CREATE:
		nRet = WndProc_Create(hwnd, (LPCREATESTRUCTA)lParam);
		if (nRet == -1)
			return -1;
		break;
	case WM_TIMER:
		WndProc_Timer(hwnd, (UINT_PTR)wParam);
		break;
	case WM_PAINT:
		WndProc_Paint(hwnd);
		break;
	case WM_KEYDOWN:
		WndProc_KeyDown(
			hwnd,
			(BYTE)wParam,
			(BYTE)(lParam & 65535),
			(BYTE)((lParam >> 16) & 255),
			(BOOL)((lParam >> 24) & 1),
			(BOOL)((lParam >> 29) & 1),
			(BOOL)((lParam >> 30) & 1),
			(BOOL)((lParam >> 31) & 1)
		);
		break;
	case WM_DESTROY:
		WndProc_Destroy(hwnd);
		break;
	}
	return DefWindowProcA(hwnd, msg, wParam, lParam);
}