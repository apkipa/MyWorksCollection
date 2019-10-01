#include "public.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSA WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = NULL;
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = g_strAppMainWndClassName;
	return RegisterClassA(&WndClass);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	ATOM atomClass;
	HWND hwMain;
	BOOL bRet;
	MSG msg;

	CoInitialize(NULL);

	atomClass = MyRegisterClass(hInstance);
	hwMain = CreateWindowA(
		ATOM2LPCSTR(atomClass),
		g_strAppMainWndName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(hwMain, nCmdShow);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1)
			return EXIT_FAILURE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();

	return (int)msg.wParam;
}