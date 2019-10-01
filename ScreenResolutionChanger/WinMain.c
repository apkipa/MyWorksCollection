#define UNICODE
#define _UNICODE

#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define SCREEN_RESOLUTION_CHANGER_CLASS_NAME L"ScreenResolutionChangerClass"
#define SCREEN_RESOLUTION_CHANGER_WINDOW_NAME L"Screen Resolution Changer"

#define BLACK_SCREEN_CLASS_NAME L"BlackScreenClass"
#define BLACK_SCREEN_WINDOW_NAME L"Black Screen"

#define BUTTON_WIDTH 20
#define BUTTON_HEIGHT 100
#define BUTTON_ROUND_SIZE 20
#define BUTTON_ARROW_LINE_WIDTH 5
#define BUTTON_ARROW_WIDTH 6
#define BUTTON_ARROW_HEIGHT 12

#define ClockToMillsecond(varClock_t) ((varClock_t) * 1000 / CLOCKS_PER_SEC)

SIZE sizeResolutionSmall, sizeResolutionBig;

POINT ptWindow;

int yOffset;

//Utilities
struct {
	BOOL bIsInDrag;
	POINT ptDrag;
}DragInformation = { .bIsInDrag = FALSE, .ptDrag = { 0 } };
void SetDrag(const POINT *pPoint) {
	if (pPoint) {
		DragInformation.bIsInDrag = TRUE;
		DragInformation.ptDrag = *pPoint;
	}
	else {
		DragInformation.bIsInDrag = FALSE;
		DragInformation.ptDrag = (POINT) { 0 };
	}
}
BOOL GetDrag(POINT *pPoint) {
	if (DragInformation.bIsInDrag) {
		if (pPoint)
			*pPoint = DragInformation.ptDrag;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

bool ChangeScreenResolution(int nWidth, int nHeight) {
	DEVMODE dmScreenSettings;
	long result;

	dmScreenSettings.dmSize = sizeof(DEVMODE);
	dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	dmScreenSettings.dmPelsWidth = nWidth;
	dmScreenSettings.dmPelsHeight = nHeight;
	result = ChangeDisplaySettings(&dmScreenSettings, 0);
	if (result == DISP_CHANGE_SUCCESSFUL) {
		ChangeDisplaySettings(&dmScreenSettings, CDS_UPDATEREGISTRY);
		return true;
	}
	else {
		ChangeDisplaySettings(NULL, 0);
		return false;
	}
}

bool GetScreenSize(SIZE *pSize) {
	if (pSize == NULL)
		return false;
	pSize->cx = GetSystemMetrics(SM_CXSCREEN);
	pSize->cy = GetSystemMetrics(SM_CYSCREEN);
	return true;
}

int GetScreenWidth(void) {
	return GetSystemMetrics(SM_CXSCREEN);
}

int GetScreenHeight(void) {
	return GetSystemMetrics(SM_CYSCREEN);
}

ATOM RegisterBlackScreenClass(HINSTANCE hInstance) {
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClass.hCursor = NULL;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = DefWindowProc;
	WndClass.lpszClassName = BLACK_SCREEN_CLASS_NAME;
	WndClass.lpszMenuName = NULL;
	WndClass.style = 0;
	return RegisterClass(&WndClass);
}

HWND CreateBlackScreenWindow(HINSTANCE hInstance) {
	return CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
		BLACK_SCREEN_CLASS_NAME,
		BLACK_SCREEN_WINDOW_NAME,
		WS_POPUP | WS_VISIBLE,
		0, 0,
		1, 1,
		NULL,
		NULL,
		hInstance,
		NULL
	);
}

/*
bool PaintBlackScreenWindowBlack(HWND hwndBlackScreen) {
	HDC hdcBlackScreen;
	RECT rtClient;

	if (hwndBlackScreen == NULL)
		return false;

	GetClientRect(hwndBlackScreen, &rtClient);
	FillRect(hdcBlackScreen, &rtClient, (HBRUSH)GetStockObject(BLACK_BRUSH));

	ReleaseDC(hwndBlackScreen, hdcBlackScreen);
}
*/

bool IsCurrentResolutionSmall(void) {
	return GetScreenWidth() < sizeResolutionBig.cx;
}

bool SwitchResolution(void) {
	if (IsCurrentResolutionSmall()) {
		ChangeScreenResolution(sizeResolutionBig.cx, sizeResolutionBig.cy);
	}
	else {
		ChangeScreenResolution(sizeResolutionSmall.cx, sizeResolutionSmall.cy);
	}
}

HRGN CombineRegion(HRGN rgn1, HRGN rgn2, int nMode) {
	HRGN rgnResult;
	rgnResult = CreateRectRgn(0, 0, 1, 1);
	CombineRgn(rgnResult, rgn1, rgn2, nMode);
	return rgnResult;
}

HRGN BuildWindowRegion(int xOffset, int yOffset) {
	HRGN rgnRoundRect, rgnRectangle, rgnResult;

	rgnRoundRect = CreateRoundRectRgn(
		xOffset, yOffset,
		xOffset + BUTTON_WIDTH + (BUTTON_ROUND_SIZE + 1) / 2, yOffset + BUTTON_HEIGHT,
		BUTTON_ROUND_SIZE,
		BUTTON_ROUND_SIZE
	);

	rgnRectangle = CreateRectRgn(
		xOffset, yOffset,
		xOffset + BUTTON_WIDTH, yOffset + BUTTON_HEIGHT
	);

	rgnResult = CombineRegion(rgnRoundRect, rgnRectangle, RGN_AND);

	DeleteObject(rgnRoundRect);
	DeleteObject(rgnRectangle);

	return rgnResult;
}

void DrawButtonArrow(HDC hdc) {
	HPEN penWhite;
	POINT ptLast;

	penWhite = CreatePen(PS_SOLID, BUTTON_ARROW_LINE_WIDTH, RGB(255, 255, 255));

	if (IsCurrentResolutionSmall()) {	//Small
		MoveToEx(hdc, (BUTTON_WIDTH - BUTTON_ARROW_WIDTH) / 2, (BUTTON_HEIGHT - BUTTON_ARROW_HEIGHT) / 2, &ptLast);
		LineTo(hdc, (BUTTON_WIDTH + BUTTON_ARROW_WIDTH) / 2, BUTTON_HEIGHT / 2);
		LineTo(hdc, (BUTTON_WIDTH - BUTTON_ARROW_WIDTH) / 2, (BUTTON_HEIGHT + BUTTON_ARROW_HEIGHT) / 2);
	}
	else {	//Big
		MoveToEx(hdc, (BUTTON_WIDTH + BUTTON_ARROW_WIDTH) / 2, (BUTTON_HEIGHT - BUTTON_ARROW_HEIGHT) / 2, &ptLast);
		LineTo(hdc, (BUTTON_WIDTH - BUTTON_ARROW_WIDTH) / 2, BUTTON_HEIGHT / 2);
		LineTo(hdc, (BUTTON_WIDTH + BUTTON_ARROW_WIDTH) / 2, (BUTTON_HEIGHT + BUTTON_ARROW_HEIGHT) / 2);
	}
	MoveToEx(hdc, ptLast.x, ptLast.y, NULL);

	DeleteObject(penWhite);
}

void MakeScreenResolutions(void) {
	GetScreenSize(&sizeResolutionBig);
	sizeResolutionSmall = (SIZE) { sizeResolutionBig.cx / 2, sizeResolutionBig.cy };
}

void WndProc_Create(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	FILE *fp;

	yOffset = (GetScreenHeight() - BUTTON_HEIGHT) / 2;

	fp = fopen("Resolutions.txt", "rb");
	if (fp == NULL) {
		MessageBox(
			hwnd,
			L"Could not read resolutions in \"Resolutions.txt\". "
			L"Assuming the current is the big resolution and half width of the current is the small resolution.",
			L"Error",
			MB_ICONERROR
		);

		MakeScreenResolutions();
	}
	else {
		if (
			fscanf(
				fp,
				" ( %ld , %ld ) , ( %ld , %ld )",
				&sizeResolutionSmall.cx, &sizeResolutionSmall.cy,
				&sizeResolutionBig.cx, &sizeResolutionBig.cy
			) != 4) {
			MessageBox(
				hwnd,
				L"Unknown resolutions format in \"Resolutions.txt\". "
				L"Assuming the current is the big resolution and half width of the current is the small resolution.",
				L"Error",
				MB_ICONERROR
			);

			MakeScreenResolutions();
		}
		else if (sizeResolutionSmall.cx > sizeResolutionBig.cx) {
			SIZE sizeTemp;
			sizeTemp = sizeResolutionSmall;
			sizeResolutionSmall = sizeResolutionBig;
			sizeResolutionBig = sizeTemp;
		}

		fclose(fp);
	}
}

void WndProc_Destroy(HWND hwnd) {
	PostQuitMessage(0);
}

void WndProc_LeftButtonDown(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	RECT rtWindow;
	POINT ptMouse;

	GetWindowRect(hwnd, &rtWindow);

	ptWindow = (POINT) { rtWindow.left, rtWindow.top };
	GetCursorPos(&ptMouse);
	SetDrag(&ptMouse);

	SetCapture(hwnd);
}

void WndProc_LeftButtonUp(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	HWND hwndBlackScreen;
	HINSTANCE hInstance;
	clock_t clkBegin;

	SetCapture(NULL);
	SetDrag(NULL);

	if (GetCursor() == LoadCursor(NULL, IDC_SIZEALL)) {	//In drag
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	else {	//Not in drag
		hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
		RegisterBlackScreenClass(hInstance);
		hwndBlackScreen = CreateBlackScreenWindow(hInstance);

		MoveWindow(hwndBlackScreen, 0, 0, GetScreenWidth(), GetScreenHeight(), FALSE);
		clkBegin = clock();
		while (ClockToMillsecond(clock() - clkBegin) <= 400) {
			SetLayeredWindowAttributes(hwndBlackScreen, 0, (BYTE)(ClockToMillsecond(clock() - clkBegin) * 255 / 400), LWA_ALPHA);
			InvalidateRect(hwndBlackScreen, NULL, FALSE);
			UpdateWindow(hwndBlackScreen);
		}

		clkBegin = clock();

		SwitchResolution();

		char msg[32];
		sprintf(msg, "Cost %d ms.", ClockToMillsecond(clock() - clkBegin));
		//MessageBoxA(hwnd, msg, NULL, 0);

		MoveWindow(hwndBlackScreen, 0, 0, GetScreenWidth(), GetScreenHeight(), FALSE);
		clkBegin = clock();
		while (ClockToMillsecond(clock() - clkBegin) <= 400) {
			SetLayeredWindowAttributes(hwndBlackScreen, 0, (BYTE)(255 - ClockToMillsecond(clock() - clkBegin) * 255 / 400), LWA_ALPHA);
			InvalidateRect(hwndBlackScreen, NULL, FALSE);
			UpdateWindow(hwndBlackScreen);
		}

		DestroyWindow(hwndBlackScreen);

		InvalidateRect(hwnd, NULL, FALSE);
	}
}

void WndProc_RightButtonDown(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	HMENU menuPopup;
	POINT ptMouse;

	GetCursorPos(&ptMouse);

	menuPopup = CreatePopupMenu();
	AppendMenu(menuPopup, MF_BYCOMMAND | MF_STRING, 1024, L"Exit");
	if (TrackPopupMenu(menuPopup, TPM_RETURNCMD, ptMouse.x, ptMouse.y, 0, hwnd, NULL) == 1024)
		PostMessage(hwnd, WM_CLOSE, 0, 0);

	DestroyMenu(menuPopup);
}

void WndProc_MouseMove(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	int yOffsetNew;
	POINT ptMouse;
	POINT ptDrag;

	if (GetDrag(&ptDrag)) {
		GetCursorPos(&ptMouse);
		yOffsetNew = ptWindow.y + (ptMouse.y - ptDrag.y);
		yOffsetNew = min(max(yOffsetNew, 0), GetScreenHeight() - BUTTON_HEIGHT);

		if (yOffsetNew != yOffset) {
			yOffset = yOffsetNew;

			SetCursor(LoadCursor(NULL, IDC_SIZEALL));
			InvalidateRect(hwnd, NULL, FALSE);
		}
	}
	else {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
}

void WndProc_Paint(HWND hwnd) {
	PAINTSTRUCT PaintStruct;
	HBRUSH brushBlue;
	HRGN rgnWindow;

	rgnWindow = BuildWindowRegion(0, 0);
	MoveWindow(hwnd, GetScreenWidth() - BUTTON_WIDTH, yOffset, BUTTON_WIDTH, BUTTON_HEIGHT, FALSE);
	SetWindowRgn(hwnd, rgnWindow, FALSE);
	DeleteObject(rgnWindow);

	BeginPaint(hwnd, &PaintStruct);

	brushBlue = CreateSolidBrush(RGB(0, 122, 204));
	rgnWindow = BuildWindowRegion(0, 0);
	FillRgn(PaintStruct.hdc, rgnWindow, brushBlue);
	DeleteObject(rgnWindow);
	DeleteObject(brushBlue);

	DrawButtonArrow(PaintStruct.hdc);

	EndPaint(hwnd, &PaintStruct);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		WndProc_Create(hwnd, (LPCREATESTRUCT)lParam);
		break;
	case WM_DESTROY:
		WndProc_Destroy(hwnd);
		break;
	case WM_LBUTTONDOWN:
		WndProc_LeftButtonDown(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_LBUTTONUP:
		WndProc_LeftButtonUp(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_RBUTTONDOWN:
		WndProc_RightButtonDown(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_MOUSEMOVE:
		WndProc_MouseMove(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_PAINT:
		WndProc_Paint(hwnd);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

ATOM My_RegisterClass(HINSTANCE hInstance) {
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = NULL;
	WndClass.hCursor = NULL;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = SCREEN_RESOLUTION_CHANGER_CLASS_NAME;
	WndClass.lpszMenuName = NULL;
	WndClass.style = 0;
	return RegisterClass(&WndClass);
}

HWND My_CreateWindow(HINSTANCE hInstance) {
	return CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST,
		SCREEN_RESOLUTION_CHANGER_CLASS_NAME,
		SCREEN_RESOLUTION_CHANGER_WINDOW_NAME,
		WS_POPUP | WS_VISIBLE,
		0, 0,
		1, 1,
		NULL,
		NULL,
		hInstance,
		NULL
	);
}

int My_CycleMessage(void) {
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	My_RegisterClass(hInstance);
	My_CreateWindow(hInstance);
	return My_CycleMessage();
}