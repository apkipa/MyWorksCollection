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

	RoInitialize(RO_INIT_MULTITHREADED);

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

	if (hwMain == NULL) {
		MessageBoxA(NULL, "Sorry, your system does not support system media transport control!", "Error", MB_ICONERROR);
		extern LRESULT lresultTemp;
		char msg[512];
		int nOffset;
		nOffset = sprintf(msg, "Error code: 0x%08x\nDetails:\n", lresultTemp);
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			lresultTemp,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			msg + nOffset,
			sizeof(msg) - nOffset,
			NULL
		);
		MessageBoxA(NULL, msg, "Error", MB_ICONERROR);
		return EXIT_FAILURE;
	}

	ShowWindow(hwMain, nCmdShow);

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1)
			return EXIT_FAILURE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	RoUninitialize();

	return (int)msg.wParam;
}