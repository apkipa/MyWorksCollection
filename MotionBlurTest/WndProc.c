#include <Windows.h>
#include <float.h>
#include <stdio.h>
#include <time.h>

#include "libeasydraw.h"
#include "worker.h"
#include "vsync.h"
#include "eval.h"

#define GetArrLen(arr) (sizeof(arr) / sizeof(*(arr)))
#define AlwaysTrue(exp) ((exp), 1)
#define AlwaysFalse(exp) ((exp), 0)

#define WM_CREATION_FINISHED (WM_USER + 1)

#define FRAMERATE_VSYNC 0

#define Timer_UpdateFrame_Id 1001

extern struct tagMotionBlurWindowGlobalData {
	unsigned int nWidth, nHeight;
	unsigned int nFrameRate;
	unsigned int nTotalFrameCount;
	double xRectangle, yRectangle;
	double wRectangle, hRectangle;
	char xRectExp[1024], yRectExp[1024];
	char wRectExp[1024], hRectExp[1024];
} WndGlobalData;

EasyDraw_Color clrBackground = EasyDraw_MakeArgb(255, 255, 255, 255), clrRectangle = EasyDraw_MakeArgb(255, 0, 122, 204);
pEasyDraw_Bitmap pBmpTemp, pBmpSurface;
pEasyDraw_Bitmap pBmpBufferList[8192];	//Max 8192

bool bStopThreadPaint;
pWorker pWorkerPaint;
int nCurFrame = 0;

unsigned int GetCurrentScreenRefreshRate(void) {
	DEVMODE dm;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	return (unsigned int)dm.dmDisplayFrequency;
}

void RenderOneFrame(size_t nFrame) {
	pEasyDraw_Bitmap pBmpCur;
	unsigned int nFrameRate;
	double xRect, yRect;
	double wRect, hRect;
	double t;

	if (pBmpBufferList[nFrame] == NULL)
		pBmpBufferList[nFrame] = EasyDraw_CreateBitmap(WndGlobalData.nWidth, WndGlobalData.nHeight);
	pBmpCur = pBmpBufferList[nFrame];

	nFrameRate = WndGlobalData.nFrameRate == FRAMERATE_VSYNC ? GetCurrentScreenRefreshRate() : WndGlobalData.nFrameRate;

	t = (double)nFrame / nFrameRate;

	EasyDraw_FillRectangle(pBmpCur, 0, 0, WndGlobalData.nWidth, WndGlobalData.nHeight, clrBackground);	//Background

	xRect = CalcExpressionWithRule(WndGlobalData.xRectExp, (char*[]) { "t" }, &t, 1);
	yRect = CalcExpressionWithRule(WndGlobalData.yRectExp, (char*[]) { "t" }, &t, 1);
	wRect = CalcExpressionWithRule(WndGlobalData.wRectExp, (char*[]) { "t" }, &t, 1);
	hRect = CalcExpressionWithRule(WndGlobalData.hRectExp, (char*[]) { "t" }, &t, 1);

	EasyDraw_FillRectangle(pBmpCur, xRect, yRect, xRect + wRect, yRect + hRect, clrRectangle);
}
void RenderOneFrame_WithBlur(size_t nFrame) {
	pEasyDraw_Bitmap pBmpCur;
	unsigned int nFrameRate;
	intptr_t nBlurHandle;
	double xRect, yRect;
	double wRect, hRect;
	double t1, t2;
	double t;

	if (pBmpBufferList[nFrame] == NULL)
		pBmpBufferList[nFrame] = EasyDraw_CreateBitmap(WndGlobalData.nWidth, WndGlobalData.nHeight);
	pBmpCur = pBmpBufferList[nFrame];

	nBlurHandle = EasyDraw_BeginMotionBlur(pBmpCur);

	nFrameRate = WndGlobalData.nFrameRate == FRAMERATE_VSYNC ? GetCurrentScreenRefreshRate() : WndGlobalData.nFrameRate;

	t1 = (double)nFrame / nFrameRate;
	t2 = (double)(nFrame + 1) / nFrameRate;

	for (int i = 0; i < 256; i++) {
		t = t1 + (t2 - t1) * i / 256;

		EasyDraw_FillRectangle(pBmpCur, 0, 0, WndGlobalData.nWidth, WndGlobalData.nHeight, clrBackground);	//Background

		xRect = CalcExpressionWithRule(WndGlobalData.xRectExp, (char*[]) { "t" }, &t, 1);
		yRect = CalcExpressionWithRule(WndGlobalData.yRectExp, (char*[]) { "t" }, &t, 1);
		wRect = CalcExpressionWithRule(WndGlobalData.wRectExp, (char*[]) { "t" }, &t, 1);
		hRect = CalcExpressionWithRule(WndGlobalData.hRectExp, (char*[]) { "t" }, &t, 1);

		EasyDraw_FillRectangle(pBmpCur, xRect, yRect, xRect + wRect, yRect + hRect, clrRectangle);

		EasyDraw_MotionBlur_AddBitmap(nBlurHandle, pBmpCur);
	}

	EasyDraw_FinishMotionBlur(nBlurHandle, pBmpCur);
}

void WorkerPaint_VsyncPaintWork_Core(void *pData) {
	clock_t clkBegin, clkEnd;
	unsigned int nPastMs;
	wchar_t strMsg[64];
	HBITMAP hbmpMem;
	bool bFirst;
	HDC hdcMem;
	HDC hdc;

	hdc = (HDC)pData;

	bFirst = pBmpBufferList[nCurFrame] == NULL;

	hdcMem = CreateCompatibleDC(hdc);
	hbmpMem = CreateCompatibleBitmap(hdc, WndGlobalData.nWidth, WndGlobalData.nHeight);
	SelectObject(hdcMem, hbmpMem);

	clkBegin = clock();
	if (bFirst)
		RenderOneFrame_WithBlur(nCurFrame);
	EasyDraw_DrawOntoDC(hdcMem, pBmpBufferList[nCurFrame], 0, 0, 0, 0, WndGlobalData.nWidth, WndGlobalData.nHeight);
	clkEnd = clock();
	nPastMs = (clkEnd - clkBegin) * 1000 / CLOCKS_PER_SEC;

	if (bFirst) {
		swprintf(
			strMsg,
			GetArrLen(strMsg),
			L"Rendering frame %u / %u... (%u ms)(vsync: %u)",
			nCurFrame,
			WndGlobalData.nTotalFrameCount,
			nPastMs,
			GetCurrentScreenRefreshRate()
		);
	}
	else {
		swprintf(
			strMsg,
			GetArrLen(strMsg),
			L"Finished. (%u / %u)(%u ms)(vsync: %u)",
			nCurFrame,
			WndGlobalData.nTotalFrameCount,
			nPastMs,
			GetCurrentScreenRefreshRate()
		);
	}
	TextOut(hdcMem, 0, 0, strMsg, wcslen(strMsg));

	BitBlt(hdc, 0, 0, WndGlobalData.nWidth, WndGlobalData.nHeight, hdcMem, 0, 0, SRCCOPY);
	DeleteObject(hdcMem);
	DeleteObject(hbmpMem);

	nCurFrame = (nCurFrame + 1) % WndGlobalData.nTotalFrameCount;
}
int WorkerPaint_VsyncPaintWork(pWorker pWk, void *pData) {
	clock_t clkBegin, clkEnd;
	HDC hdc;

	hdc = GetDC((HWND)pData);
	SetBkMode(hdc, TRANSPARENT);

	while (!IsWorkerStopping(pWk)) {
		if (pBmpSurface != NULL)
			PerformVSyncPaint(WorkerPaint_VsyncPaintWork_Core, hdc);
	}

	ReleaseDC((HWND)pData, hdc);

	return 0;
}
int WorkerPaint_TimerPaintWork(pWorker pWk, void *pData) {
	clock_t clkBegin, clkEnd;
	unsigned int nPastMs;
	wchar_t strMsg[64];
	bool bFirst;
	HDC hdc;

	hdc = GetDC((HWND)pData);
	SetBkMode(hdc, TRANSPARENT);

	if (pBmpSurface != NULL) {
		bFirst = pBmpBufferList[nCurFrame] == NULL;

		clkBegin = clock();
		if (bFirst)
			RenderOneFrame_WithBlur(nCurFrame);
		EasyDraw_DrawOntoDC(hdc, pBmpBufferList[nCurFrame], 0, 0, 0, 0, WndGlobalData.nWidth, WndGlobalData.nHeight);
		clkEnd = clock();
		nPastMs = (clkEnd - clkBegin) * 1000 / CLOCKS_PER_SEC;

		if (bFirst) {
			swprintf(
				strMsg,
				GetArrLen(strMsg),
				L"Rendering frame %u / %u... (%u ms)",
				nCurFrame,
				WndGlobalData.nTotalFrameCount,
				nPastMs
			);
		}
		else {
			swprintf(
				strMsg,
				GetArrLen(strMsg),
				L"Finished. (%u / %u)(%u ms)",
				nCurFrame,
				WndGlobalData.nTotalFrameCount,
				nPastMs
			);
		}
		TextOut(hdc, 0, 0, strMsg, wcslen(strMsg));

		nCurFrame = (nCurFrame + 1) % WndGlobalData.nTotalFrameCount;
	}

	ReleaseDC((HWND)pData, hdc);

	return 0;
}

void WndProc_Create(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	memset(pBmpBufferList, 0, sizeof(pBmpBufferList));
}

void WndProc_CreationFinished(HWND hwnd) {
	pBmpSurface = EasyDraw_CreateBitmap(WndGlobalData.nWidth, WndGlobalData.nHeight);
	pBmpTemp = EasyDraw_CreateBitmap(WndGlobalData.nWidth, WndGlobalData.nHeight);

	//for (size_t i = 0; i < WndGlobalData.nTotalFrameCount; i++)
	//	pBmpBufferList[i] = EasyDraw_CreateBitmap(WndGlobalData.nWidth, WndGlobalData.nHeight);

	pWorkerPaint = CreateWorker();

	if (WndGlobalData.nFrameRate == FRAMERATE_VSYNC) {
		SetWorkerWork(pWorkerPaint, WorkerPaint_VsyncPaintWork);
		StartWorkerWork(pWorkerPaint, hwnd, WORKER_WORK_ASYNC);
	}
	else {
		SetWorkerWork(pWorkerPaint, WorkerPaint_TimerPaintWork);
		SetTimer(hwnd, Timer_UpdateFrame_Id, 1000 / WndGlobalData.nFrameRate - 1, NULL);
	}
}

void WndProc_Destroy(HWND hwnd) {
	StopWorkerWork(pWorkerPaint, true);
	DestroyWorker(pWorkerPaint);
	if (WndGlobalData.nFrameRate != FRAMERATE_VSYNC)
		KillTimer(hwnd, Timer_UpdateFrame_Id);

	for (size_t i = 0; i < WndGlobalData.nTotalFrameCount; i++)
		EasyDraw_DestroyBitmap(pBmpBufferList[i]);

	EasyDraw_DestroyBitmap(pBmpTemp);
	EasyDraw_DestroyBitmap(pBmpSurface);

	PostQuitMessage(0);
}

void WndProc_Paint(HWND hwnd) {
	ValidateRect(hwnd, NULL);
}

void WndProc_Timer(HWND hwnd, UINT_PTR nTimerId) {
	switch (nTimerId) {
	case Timer_UpdateFrame_Id:
		StartWorkerWork(pWorkerPaint, hwnd, WORKER_WORK_SYNC);
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		WndProc_Create(hwnd, (LPCREATESTRUCT)lParam);
		break;
	case WM_CREATION_FINISHED:
		WndProc_CreationFinished(hwnd);
		break;
	case WM_DESTROY:
		WndProc_Destroy(hwnd);
		break;
	case WM_PAINT:
		WndProc_Paint(hwnd);
		break;
	case WM_TIMER:
		WndProc_Timer(hwnd, wParam);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}