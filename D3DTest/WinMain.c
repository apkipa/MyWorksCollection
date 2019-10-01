#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#define D3D_TEST_WINDOW_CLASS L"Direct3DTestWindowClass"
#define D3D_TEST_WINDOW_TITLE L"Direct3D Test"

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
	wc.lpszClassName = D3D_TEST_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	return RegisterClass(&wc);
}

HWND MyCreateWnd(HINSTANCE hInstance) {
	return CreateWindow(
		D3D_TEST_WINDOW_CLASS,
		D3D_TEST_WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
	MyRegisterWndClass(hInstance);
	MyCreateWnd(hInstance);
	return MyMsgLoop();
}