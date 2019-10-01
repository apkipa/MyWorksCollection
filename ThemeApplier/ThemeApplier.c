#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>





#define CLR_I FOREGROUND_INTENSITY
#define CLR_R FOREGROUND_RED
#define CLR_G FOREGROUND_GREEN
#define CLR_B FOREGROUND_BLUE

WORD nAttriQueue[1024];
int nAttriPtrFront = 0;
void PushColor(WORD nNewClr) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	nAttriQueue[nAttriPtrFront++] = csbi.wAttributes;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nNewClr);
}
void PopColor(void) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), nAttriQueue[--nAttriPtrFront]);
}

void PrintInfo(const char *fmt, ...) {
	char buf[1024];
	va_list vl;

	va_start(vl, fmt);
	vsprintf(buf, fmt, vl);
	va_end(vl);

	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	putchar('[');
	PopColor();
	printf("%s", "Info");
	PopColor();
	printf("] %s\n", buf);
	PopColor();
}

void PrintWarning(const char *fmt, ...) {
	char buf[1024];
	va_list vl;

	va_start(vl, fmt);
	vsprintf(buf, fmt, vl);
	va_end(vl);

	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_R | CLR_G);
	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	putchar('[');
	PopColor();
	printf("%s", "Warning");
	PopColor();
	printf("] %s\n", buf);
	PopColor();
}

void PrintError(const char *fmt, ...) {
	char buf[1024];
	va_list vl;

	va_start(vl, fmt);
	vsprintf(buf, fmt, vl);
	va_end(vl);

	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	PushColor(CLR_I | CLR_R);
	PushColor(CLR_I | CLR_R | CLR_G | CLR_B);
	putchar('[');
	PopColor();
	printf("%s", "Error");
	PopColor();
	printf("] %s\n", buf);
	PopColor();
}





HRESULT hresLastError;

#define GetInterfaceVTable(pi) ( *(DWORD_PTR*)(pi) )
#define GetInterfaceMethod(pi, index) ( *((DWORD_PTR*)GetInterfaceVTable(pi) + (index)) )

bool FileExistsA(LPCSTR strPath) {
  DWORD dwAttrib = GetFileAttributesA(strPath);
  return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

#if 0
typedef HRESULT (WINAPI *pfSetSystemVisualStyle_t)(
	LPCWSTR pszVisualStyleFile,
	LPCWSTR pszColorParam,
	LPCWSTR pszSizeParam,
	DWORD dwFlags
);

bool ApplyNewVisualStyle(const char *path) {
	static pfSetSystemVisualStyle_t pfSetSystemVisualStyle = NULL;
	wchar_t wstrBuf[1024];

	if (pfSetSystemVisualStyle == NULL) {
		HMODULE hmUxtheme;

		hmUxtheme = GetModuleHandleA("uxtheme.dll");
		if (hmUxtheme == NULL)
			hmUxtheme = LoadLibraryA("uxtheme.dll");
		if (hmUxtheme == NULL)
			return false;

		pfSetSystemVisualStyle = (pfSetSystemVisualStyle_t)GetProcAddress(hmUxtheme, MAKEINTRESOURCE(65));
		if (pfSetSystemVisualStyle == NULL)
			return false;
	}

	strcpy(path, "C:\\Windows\\Resources\\Themes\\aero.theme");

	MultiByteToWideChar(CP_ACP, 0, path, -1, wstrBuf, 1024);

	return SUCCEEDED( hresLastError = pfSetSystemVisualStyle(wstrBuf, L"Sky", L"NormalSize", 0) );
}
#else
bool ApplyNewVisualStyle(const char *path) {
	wchar_t wstrBuf[1024];

	// NOTE: Tested working for Windows 7

	MultiByteToWideChar(CP_ACP, 0, path, -1, wstrBuf, 1024);

	// CLSID_ThemeManagerShared: from themeui.dll
	GUID clsid = { 0xc04b329e, 0x5823, 0x4415, { 0x9c, 0x93, 0xba, 0x44, 0x68, 0x89, 0x47, 0xb0 } };
	// IID_IThemeManagerShared: from CLSID_ThemeManagerShared
	//   (Can be retrieved through IID_IUnknown since it is the first QITAB element)
	GUID iid = { 0x0646ebbe, 0xc1b7, 0x4045, { 0x8f, 0xd0, 0xff, 0xd6, 0x5d, 0x3f, 0xc7, 0x92 } };
	void *pi;

	hresLastError = CoCreateInstance(&clsid, NULL, CLSCTX_ALL, &iid, &pi);
	if ( FAILED(hresLastError) )
		return false;

	typedef HRESULT (WINAPI *pfIThemeManagerShared_Release_t)(void *pi);
	typedef HRESULT (WINAPI *pfIThemeManagerShared_ApplyTheme_t)(void *pi, const wchar_t *strThemePath);

	/*	Disassembled methods in IThemeManagerShared:
		0.    QueryInterface(REFIID, void**)
		1.    AddRef(void)
		2.    Release(void)
		3.    get_CurrentTheme(IThemeShared**)
		4.    ApplyTheme(BSTR)
		5.    (UNKNOWN) CNamespaceWalkFilter::LeaveFolder(IShellFolder*, const struct _ITEMID_CHILD*)
	*/

	pfIThemeManagerShared_Release_t pfIThemeManagerShared_Release = (pfIThemeManagerShared_Release_t)GetInterfaceMethod(pi, 2);
	pfIThemeManagerShared_ApplyTheme_t pfIThemeManagerShared_ApplyTheme = (pfIThemeManagerShared_ApplyTheme_t)GetInterfaceMethod(pi, 4);

	hresLastError = pfIThemeManagerShared_ApplyTheme(pi, wstrBuf);
	pfIThemeManagerShared_Release(pi);

	return SUCCEEDED(hresLastError);
}
#endif

const char* LocateVisualTheme(const char *name, char *path) {
	sprintf(path, "%s\\%s.theme", "C:\\Windows\\Resources\\Themes", name);
	if ( FileExistsA(path) )
		return path;
	sprintf(path, "%s\\%s", "C:\\Windows\\Resources\\Themes", name);
	if ( FileExistsA(path) )
		return path;
	sprintf(path, "%s\\%s.theme", "C:\\Windows\\Resources\\Easy of Access Themes", name);
	if ( FileExistsA(path) )
		return path;
	sprintf(path, "%s\\%s", "C:\\Windows\\Resources\\Easy of Access Themes", name);
	if ( FileExistsA(path) )
		return path;
	return NULL;
}

int main(int argc, char *argv[]) {
	char strBuf[1024];

	CoInitialize(NULL);

	if (argc != 2) {
		PrintError("Arguments too many or too few");
		goto AbnormalExit;
	}

	if ( LocateVisualTheme(argv[1], strBuf) == NULL ) {
		PrintError("Theme \"%s\" not found", argv[1]);
		goto AbnormalExit;
	}

	if ( !ApplyNewVisualStyle(strBuf) ) {
		PrintError("Could not apply theme \"%s\"(\"%s\") (0x%08x)", argv[1], strBuf, hresLastError);
		goto AbnormalExit;
	}

	CoUninitialize();
	return EXIT_SUCCESS;
AbnormalExit:
	CoUninitialize();
	return EXIT_FAILURE;
}