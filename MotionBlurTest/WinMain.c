#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>

#define MOTION_BLUR_TEST_WINDOW_CLASS L"MotionBlurTestWindowClass"
#define MOTION_BLUR_TEST_WINDOW_TITLE L"Motion Blur Test Window"

#define WM_CREATION_FINISHED (WM_USER + 1)

struct tagMotionBlurWindowGlobalData {
	unsigned int nWidth, nHeight;
	unsigned int nFrameRate;
	unsigned int nTotalFrameCount;
	double xRectangle, yRectangle;
	double wRectangle, hRectangle;
	char xRectExp[1024], yRectExp[1024];
	char wRectExp[1024], hRectExp[1024];
} WndGlobalData;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

ATOM MyRegisterWndClass(HINSTANCE hInstance) {
	WNDCLASS wc;

	wc.hInstance = hInstance;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = MOTION_BLUR_TEST_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	return RegisterClass(&wc);
}

HWND MyCreateWnd(HINSTANCE hInstance) {
	return CreateWindow(
		MOTION_BLUR_TEST_WINDOW_CLASS,
		MOTION_BLUR_TEST_WINDOW_TITLE,
		(WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX)) | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
}

int MyMsgLoop(void) {
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	RECT rtWindow;
	HWND hwnd;

	MyRegisterWndClass(hInstance);
	hwnd = MyCreateWnd(hInstance);

	ShowWindow(hwnd, SW_HIDE);

	AllocConsole();
	freopen("CONIN$", "r+t", stdin);
	freopen("CONOUT$", "w+t", stdout);

	printf("Step 1 / 3:\n");
	printf("Enter client size (separate with space)(set any to 0 to use original size): ");
	scanf("%u%u", &WndGlobalData.nWidth, &WndGlobalData.nHeight);
	if (WndGlobalData.nWidth != 0 && WndGlobalData.nHeight != 0) {
		rtWindow.left = 0;
		rtWindow.top = 0;
		rtWindow.right = WndGlobalData.nWidth;
		rtWindow.bottom = WndGlobalData.nHeight;
		AdjustWindowRect(&rtWindow, GetWindowLong(hwnd, GWL_STYLE), false);
		SetWindowPos(hwnd, NULL, 0, 0, rtWindow.right - rtWindow.left, rtWindow.bottom - rtWindow.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	//printf("Step 2 / 2:\n");
	//printf("Enter test recgangle info (x, y, width, height)(separate with space): ");
	//scanf("%lf%lf", &WndGlobalData.xRectangle, &WndGlobalData.yRectangle);
	//scanf("%lf%lf", &WndGlobalData.wRectangle, &WndGlobalData.hRectangle);

	printf("Step 2 / 3:\n");
	printf("Enter test recgangle info (x, y, width, height)(can be expressions, use 't' as time)(separate with space): ");
	scanf("%s%s", &WndGlobalData.xRectExp, &WndGlobalData.yRectExp);
	scanf("%s%s", &WndGlobalData.wRectExp, &WndGlobalData.hRectExp);

	printf("Step 3 / 3:\n");
	printf("Enter frame rate and total frame count (separate with space)(set frame rate to 0 for vsync): ");
	scanf("%u%u", &WndGlobalData.nFrameRate, &WndGlobalData.nTotalFrameCount);

	fclose(stdout);
	fclose(stdin);
	FreeConsole();

	SendMessage(hwnd, WM_CREATION_FINISHED, 0, 0);

	ShowWindow(hwnd, SW_SHOW);

	return MyMsgLoop();
}