#include <Windows.h>
#include <stdio.h>
#include <time.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9")

#include "libeasydraw.h"
#include "d3dhelper.h"
#include "timer.h"

#define AlignUpward(n, align) (((uintptr_t)(n) + (align) - 1) & ~(uintptr_t)((align) - 1))

EasyDraw_Color clrWhite = EasyDraw_MakeArgb(255, 255, 255, 255), clrBlack = EasyDraw_MakeArgb(255, 0, 0, 0);
EasyDraw_Color clrBlue = EasyDraw_MakeArgb(255, 0, 122, 204), clrOrange = EasyDraw_MakeArgb(255, 255, 127, 39);

pEasyDraw_Bitmap pBmpSurface;
pHRTimer pTimer;

#define ClockToMs(clk) ((clk) * 1000 / CLOCKS_PER_SEC)
#define fClockToMs(clk) ((clk) * 1000.0 / CLOCKS_PER_SEC)

pD3DDeviceData pd3ddd;

void WndProc_Create(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	RECT rtClient;

	GetClientRect(hwnd, &rtClient);

	pTimer = CreateHighResolutionTimer();
	pd3ddd = InitD3DData(hwnd);
	pBmpSurface = EasyDraw_CreateBitmap(AlignUpward(rtClient.right, 32), rtClient.bottom);
}

void WndProc_Destroy(HWND hwnd) {
	DestroyHighResolutionTimer(pTimer);
	EasyDraw_DestroyBitmap(pBmpSurface);
	UninitD3DData(pd3ddd);

	PostQuitMessage(0);
}

void WndProc_MouseMove(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		InvalidateRect(hwnd, NULL, false);
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
	InvalidateRect(hwnd, NULL, false);
}

void WndProc_KeyUp(
	HWND hwnd,
	BYTE byteVirtualKeyCode,
	BYTE byteRepeatCount,
	BYTE byteScanCode,
	BOOL bIsExtended,
	BOOL bContextCode,
	BOOL bPreviousKeyState,
	BOOL bTransitionState
) {
	if (byteVirtualKeyCode == VK_SPACE)
		InvalidateRect(hwnd, NULL, false);
}

void WndProc_Size(HWND hwnd, int nResizingRequested, int nClientWidth, int nClientHeight) {
	D3DLOCKED_RECT d3dRectLocked;

	ResetD3DData(pd3ddd, false);
	D3DBackBuffer_LockRect(pd3ddd, NULL, &d3dRectLocked);
	D3DBackBuffer_UnlockRect(pd3ddd);

	EasyDraw_DestroyBitmap(pBmpSurface);
	pBmpSurface = EasyDraw_CreateBitmap(d3dRectLocked.Pitch / 4, nClientHeight);
}

void WndProc_Paint(HWND hwnd) {
	double fTimeSys, fTimeUsr, fTimeVideo;
	static bool bSectionEntered = false;
	D3DLOCKED_RECT d3dRectLocked;
	static wchar_t msg[128];
	RECT rtClient;
	POINT ptMouse;
	int nChoice;

	GetClientRect(hwnd, &rtClient);

	BeginHighResolutionTimer(pTimer);

	if (GetAsyncKeyState('L') & 0x8000) {
		wchar_t msg[128];
		Sleep(1000);
		StopHighResolutionTimer(pTimer);
		swprintf(msg, 128, L"Program slept 1000ms, measured %.0fms", fGetHighResolutionTimerValue(pTimer) * 1000);
		MessageBox(hwnd, msg, L"Time measure test", MB_ICONINFORMATION);
	}

	if (bSectionEntered)
		return;

	if (!D3DDevice_IsDeviceOK(pd3ddd)) {
		bSectionEntered = true;

		if (!ResetD3DData(pd3ddd, true)) {
			do {
				nChoice = MessageBox(
					hwnd,
					L"A fatal Direct3D error occurred.\n"
					L"Press \"OK\" to retry, or \"Cancel\" to terminate the program.",
					L"Error",
					MB_ICONERROR | MB_OKCANCEL
				);
				if (nChoice == IDCANCEL)
					FatalExit(0);
				pd3ddd = InitD3DData(hwnd);
			} while (pd3ddd == NULL);
		}

		bSectionEntered = false;
	}

	D3DBackBuffer_LockRect(pd3ddd, NULL, &d3dRectLocked);

	//pBmpSurface = EasyDraw_CreateBitmap(d3dRectLocked.Pitch / 4, rtClient.bottom);
	if (EasyDraw_GetBitmapWidth(pBmpSurface) != d3dRectLocked.Pitch / 4) {
		wchar_t msg[128];
		wsprintf(
			msg,
			L"Mismatch!\nEasyDraw_GetBitmapWidth(pBmpSurface) = %d\nd3dRectLocked.Pitch = %d => %d",
			EasyDraw_GetBitmapWidth(pBmpSurface),
			d3dRectLocked.Pitch,
			d3dRectLocked.Pitch / 4
		);
		FatalAppExit(0, msg);
	}

	fTimeSys = fGetHighResolutionTimerValue(pTimer);

	GetCursorPos(&ptMouse);
	ScreenToClient(hwnd, &ptMouse);
	EasyDraw_fFillRectangle(pBmpSurface, 0.5 + ptMouse.x, 0.5 + ptMouse.y, 300.5 + ptMouse.x, 300.5 + ptMouse.y, &EasyDraw_MakeBrush(clrOrange, 255 / 8));
	if (GetAsyncKeyState('C') & 0x8000)
		EasyDraw_fFillRectangle(pBmpSurface, 0, 0, d3dRectLocked.Pitch / 4, rtClient.bottom, &EasyDraw_MakeBrush(clrWhite, 255));
	EasyDraw_DrawBasicTextW(pBmpSurface, 0, 0, msg, 255);

	fTimeUsr = fGetHighResolutionTimerValue(pTimer) - fTimeSys;

	EasyDraw_CopyToMemory(d3dRectLocked.pBits, pBmpSurface);
	fTimeVideo = fGetHighResolutionTimerValue(pTimer) - fTimeSys - fTimeUsr;

	//EasyDraw_DestroyBitmap(pBmpSurface);

	D3DBackBuffer_UnlockRect(pd3ddd);

	D3DDevice_Present(pd3ddd);

	StopHighResolutionTimer(pTimer);
	fTimeSys = fGetHighResolutionTimerValue(pTimer) - fTimeUsr - fTimeVideo;

	//wchar_t msg[128];
	//wsprintf(msg, L"Direct3D Test - pitch = %d, address = 0x%p, time = %dms", d3dRectLocked.Pitch, d3dRectLocked.pBits, clkEnd - clkBegin);
	//SetWindowText(hwnd, msg);
	swprintf(
		msg,
		128,
		//L"Direct3D Test - Time cost = %.0fms(usr %.0fms, sys %.0fms, video %.0fms)",
		L"time cost = %.0fms(usr %.0fms, sys %.0fms, video %.0fms)    ",
		(fTimeSys + fTimeUsr + fTimeVideo) * 1000,
		fTimeUsr * 1000,
		fTimeSys * 1000,
		fTimeVideo * 1000
	);
	//SetWindowText(hwnd, msg);

	//Sleep(10);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		WndProc_Create(hwnd, (LPCREATESTRUCT)lParam);
		break;
	case WM_DESTROY:
		WndProc_Destroy(hwnd);
		break;
	case WM_MOUSEMOVE:
		WndProc_MouseMove(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
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
	case WM_KEYUP:
		WndProc_KeyUp(
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
	case WM_PAINT:
		WndProc_Paint(hwnd);
		break;
	case WM_SIZE:
		WndProc_Size(hwnd, (int)wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}