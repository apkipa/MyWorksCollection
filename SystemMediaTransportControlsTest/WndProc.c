#include "public.h"

#define TIMER_ID_UpdateData 10001

#define WM_USER_Play (WM_USER + 0)
#define WM_USER_Pause (WM_USER + 1)
#define WM_USER_Stop (WM_USER + 2)
#define WM_USER_Previous (WM_USER + 3)
#define WM_USER_Next (WM_USER + 4)

static inline int GetTextHeight(HDC hdc) {
	TEXTMETRICA tm;
	GetTextMetricsA(hdc, &tm);
	return tm.tmHeight;
}

#define lengthof(arr) ( sizeof(arr) / sizeof(*(arr)) )
#define FastWindowsCreateStringReference(strLiteral, phstrHdr, phstr)	\
	WindowsCreateStringReference(strLiteral, lengthof(strLiteral) - 1, phstrHdr, phstr)

static wchar_t strSongsList[32][MAX_PATH];
static int nSongsCount = 0;
int nCurSong = 0;
static bool bFirst = true;

//#define EqualGUID(id1, id2) ( memcmp(&(id1), &(id2), sizeof(GUID)) == 0 )
#define EqualGUID(id1, id2) IsEqualGUID(&(id1), &(id2))

typedef struct tagCustomCallback1Stru {
	__FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs i;
	int nCnt;
	uintptr_t nPtr1;
	uintptr_t nPtr2;
} CustomCallback1Stru;

HRESULT STDMETHODCALLTYPE callback1_QueryInterface(
	__FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs *This,
	REFIID riid,
	void **ppvObject
) {
	//Condition: No proxy

	//Accept IID___FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs
	//Accept IID_IUnknown
	//DONT accept IID_INoMarshal
	//DONT accept IdentityUnmarshal

	if (
		EqualGUID(*riid, IID___FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs) ||
		EqualGUID(*riid, IID_IUnknown)
	) {
		InterlockedIncrement(&((CustomCallback1Stru*)This)->nCnt);
		*ppvObject = (void*)This;
		return S_OK;
	}

	char strBuf[1024];
	sprintf(strBuf, "%" PRIGUID, PRIGUID_Arg(*riid));
	//MessageBoxA((HWND)((CustomCallback1Stru*)This)->nPtr1, strBuf, "QueryInterface IID data", MB_ICONWARNING);
	*ppvObject = NULL;

	return E_NOTIMPL;
}
ULONG STDMETHODCALLTYPE callback1_AddRef(
	__FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs *This
) {
	return InterlockedIncrement(&((CustomCallback1Stru*)This)->nCnt);
}
ULONG STDMETHODCALLTYPE callback1_Release(
	__FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs *This
) {
	int nRet = InterlockedDecrement(&((CustomCallback1Stru*)This)->nCnt);
	if (nRet == 0)
		free(This);
	return nRet;
}
HRESULT STDMETHODCALLTYPE callback1_Invoke(
	__FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs *This,
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *sender,
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsButtonPressedEventArgs **e
) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsButtonPressedEventArgs *pe;
	__x_ABI_CWindows_CMedia_CSystemMediaTransportControlsButton nButton;
	HRESULT nRet;
	char *strMsg;
	/*
	enum __x_ABI_CWindows_CMedia_CSystemMediaTransportControlsButton
	{
		SystemMediaTransportControlsButton_Play = 0,
		SystemMediaTransportControlsButton_Pause = 1,
		SystemMediaTransportControlsButton_Stop = 2,
		SystemMediaTransportControlsButton_Record = 3,
		SystemMediaTransportControlsButton_FastForward = 4,
		SystemMediaTransportControlsButton_Rewind = 5,
		SystemMediaTransportControlsButton_Next = 6,
		SystemMediaTransportControlsButton_Previous = 7,
		SystemMediaTransportControlsButton_ChannelUp = 8,
		SystemMediaTransportControlsButton_ChannelDown = 9,
	};
	*/
	pe = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsButtonPressedEventArgs*)e;
	nRet = __x_ABI_CWindows_CMedia_CISystemMediaTransportControlsButtonPressedEventArgs_get_Button(pe, &nButton);

	if (FAILED(nRet))
		return nRet;

	switch (nButton) {
	case SystemMediaTransportControlsButton_Play:
		strMsg = "You pressed Play button.";
		PostMessage((HWND)((CustomCallback1Stru*)This)->nPtr1, WM_USER_Play, 0, 0);
		break;
	case SystemMediaTransportControlsButton_Pause:
		strMsg = "You pressed Pause button.";
		PostMessage((HWND)((CustomCallback1Stru*)This)->nPtr1, WM_USER_Pause, 0, 0);
		break;
	case SystemMediaTransportControlsButton_Stop:
		strMsg = "You pressed Stop button.";
		PostMessage((HWND)((CustomCallback1Stru*)This)->nPtr1, WM_USER_Stop, 0, 0);
		break;
	case SystemMediaTransportControlsButton_Previous:
		strMsg = "You pressed Previous button.";
		PostMessage((HWND)((CustomCallback1Stru*)This)->nPtr1, WM_USER_Previous, 0, 0);
		break;
	case SystemMediaTransportControlsButton_Next:
		strMsg = "You pressed Next button.";
		PostMessage((HWND)((CustomCallback1Stru*)This)->nPtr1, WM_USER_Next, 0, 0);
		break;
	default:
		strMsg = "You pressed a unknown button.";
		break;
	}

	//MessageBoxA((HWND)((CustomCallback1Stru*)This)->nPtr1, strMsg, "Information", MB_ICONINFORMATION);

	return S_OK;
}

struct __FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgsVtbl callback1Vtbl = {
	callback1_QueryInterface,
	callback1_AddRef,
	callback1_Release,
	callback1_Invoke
};

CustomCallback1Stru callback1 = {
	&callback1Vtbl, 1	//Set count to 1 in order not to mistakenly free the callback structure
};

EventRegistrationToken callback1_Token;

LRESULT lresultTemp;

LRESULT WndProc_NonClientCreate(HWND hwnd, LPCREATESTRUCTA lpCreateStruct) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsDisplayUpdater *piSmtcdu;
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;
	ISystemMediaTransportControlsInterop *piSmtci;

	HSTRING_HEADER hstrHdrClass;
	HSTRING hstrClass;

	callback1.nPtr1 = (uintptr_t)hwnd;

	if (FAILED(lresultTemp = FastWindowsCreateStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls, &hstrHdrClass, &hstrClass)))
		return FALSE;
	if (FAILED(lresultTemp = RoGetActivationFactory(hstrClass, &IID_ISystemMediaTransportControlsInterop, (void**)&piSmtci)))
		return FALSE;

	if (FAILED(lresultTemp = ISystemMediaTransportControlsInterop_GetForWindow(piSmtci, hwnd, &IID___x_ABI_CWindows_CMedia_CISystemMediaTransportControls, (void**)&piSmtc)))
		return FALSE;

	ISystemMediaTransportControlsInterop_Release(piSmtci);

	if (FAILED(lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_IsEnabled(piSmtc, true)))
		return FALSE;

	if (FAILED(lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_IsPlayEnabled(piSmtc, true)))
		return FALSE;
	if (FAILED(lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_IsPauseEnabled(piSmtc, true)))
		return FALSE;
	if (FAILED(lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_IsStopEnabled(piSmtc, true)))
		return FALSE;
	if (FAILED(lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_IsPreviousEnabled(piSmtc, true)))
		return FALSE;
	if (FAILED(lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_IsNextEnabled(piSmtc, true)))
		return FALSE;

	if (FAILED(lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_get_DisplayUpdater(piSmtc, &piSmtcdu)))
		return FALSE;
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsDisplayUpdater_put_Type(piSmtcdu, MediaPlaybackType_Music);
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsDisplayUpdater_Release(piSmtcdu);

	//__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_PlaybackStatus(piSmtc, MediaPlaybackStatus_Stopped);

	if (FAILED(
		lresultTemp = __x_ABI_CWindows_CMedia_CISystemMediaTransportControls_add_ButtonPressed(
			piSmtc,
			(__FITypedEventHandler_2_Windows__CMedia__CSystemMediaTransportControls_Windows__CMedia__CSystemMediaTransportControlsButtonPressedEventArgs*)&callback1,
			&callback1_Token
		))
	) {
		return FALSE;
	}

	extern bool test(void);
	lresultTemp = test();
	//return lresultTemp == S_OK;

	SetWindowUserData(hwnd, (LONG_PTR)piSmtc);

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

void WndProc_Paint(HWND hwnd) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;
	__x_ABI_CWindows_CMedia_CMediaPlaybackStatus nStatus;
	wchar_t strBuf[4096];
	int nTextHeight;
	PAINTSTRUCT ps;
	RECT rtClient;
	int nOffset;

	GetClientRect(hwnd, &rtClient);
	piSmtc = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControls*)GetWindowUserData(hwnd);

	BeginPaint(hwnd, &ps);
	nTextHeight = GetTextHeight(ps.hdc);

	nOffset = wsprintfW(strBuf, L"Enjoy Windows 10 features! :)\n");
	nOffset += wsprintfW(strBuf + nOffset, L"System Media Transport Controls Panel:\n");
	if (SUCCEEDED(__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_get_PlaybackStatus(piSmtc, &nStatus))) {
		wchar_t *strStatus;
		switch (nStatus) {
		case MediaPlaybackStatus_Closed:
			strStatus = L"Closed";
			break;
		case MediaPlaybackStatus_Changing:
			strStatus = L"Changing";
			break;
		case MediaPlaybackStatus_Stopped:
			strStatus = L"Stopped";
			break;
		case MediaPlaybackStatus_Playing:
			strStatus = L"Playing";
			break;
		case MediaPlaybackStatus_Paused:
			strStatus = L"Paused";
			break;
		default:
			strStatus = L"Unknown";
			break;
		}
		nOffset += wsprintfW(strBuf + nOffset, L"Current status: %s\n", strStatus);
	}
	else {
		nOffset += wsprintfW(strBuf + nOffset, L"Cannot retrieve playback information from interface!\n");
	}

	if (bFirst) {
		nOffset = wsprintfW(strBuf, L"Please wait while program gathers music information from your disk.\n");
	}
	else {
		nOffset += wsprintfW(strBuf + nOffset, L"Found %d songs.\n", nSongsCount);
	}

	if (nSongsCount > 0) {
		nOffset += wsprintfW(strBuf + nOffset, L"Current song: (%d) %s\n", nCurSong + 1, strSongsList[nCurSong]);
	}

	DrawTextW(ps.hdc, strBuf, -1, &rtClient, 0);

	if (bFirst) {
		WIN32_FIND_DATAW fd;
		HANDLE hFind;

		//*.wav
		hFind = FindFirstFileW(L"*.wav", &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				wcscpy(strSongsList[nSongsCount++], fd.cFileName);
			} while (FindNextFileW(hFind, &fd));
			FindClose(hFind);
		}

		//*.mp3
		hFind = FindFirstFileW(L"*.mp3", &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				wcscpy(strSongsList[nSongsCount++], fd.cFileName);
			} while (FindNextFileW(hFind, &fd));
			FindClose(hFind);
		}

		bFirst = false;
	}

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

}

void WndProc_Destroy(HWND hwnd) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;

	piSmtc = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControls*)GetWindowUserData(hwnd);

	KillTimer(hwnd, TIMER_ID_UpdateData);

	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_remove_ButtonPressed(piSmtc, callback1_Token);
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_IsEnabled(piSmtc, false);
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_Release(piSmtc);

	PostQuitMessage(EXIT_SUCCESS);
}

void WndProc_User_Play(HWND hwnd) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsDisplayUpdater *piSmtcdu;
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;
	__x_ABI_CWindows_CMedia_CIMusicDisplayProperties *piMdp;
	HSTRING_HEADER hstrHdrBuf;
	wchar_t strBuf[1024];
	HSTRING hstrBuf;

	piSmtc = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControls*)GetWindowUserData(hwnd);

	if (nSongsCount == 0) {
		MessageBoxA(hwnd, "You cannot play songs!", "Error", MB_ICONERROR);
	}
	else {
		wsprintfW(strBuf, L"open \"%s\" alias MySound1", strSongsList[nCurSong]);
		mciSendStringW(strBuf, NULL, 0, NULL);
		mciSendStringA("play MySound1", NULL, 0, NULL);

		__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_PlaybackStatus(piSmtc, MediaPlaybackStatus_Playing);

		if (SUCCEEDED(__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_get_DisplayUpdater(piSmtc, &piSmtcdu))) {
			if (SUCCEEDED(__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsDisplayUpdater_get_MusicProperties(piSmtcdu, &piMdp))) {
				WindowsCreateStringReference(
					strBuf,
					wsprintfW(strBuf, L"%d. %s", nCurSong + 1, strSongsList[nCurSong]),
					&hstrHdrBuf,
					&hstrBuf
				);
				__x_ABI_CWindows_CMedia_CIMusicDisplayProperties_put_Title(piMdp, hstrBuf);

				WindowsCreateStringReference(
					strBuf,
					wsprintfW(strBuf, L"SystemMediaTransportControlsTest"),
					&hstrHdrBuf,
					&hstrBuf
				);
				__x_ABI_CWindows_CMedia_CIMusicDisplayProperties_put_Artist(piMdp, hstrBuf);
				//__x_ABI_CWindows_CMedia_CIMusicDisplayProperties_put_AlbumArtist(piMdp, hstrBuf);

				__x_ABI_CWindows_CMedia_CIMusicDisplayProperties_Release(piMdp);
			}

			__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsDisplayUpdater_Update(piSmtcdu);
			__x_ABI_CWindows_CMedia_CISystemMediaTransportControlsDisplayUpdater_Release(piSmtcdu);
		}
	}
}

void WndProc_User_Pause(HWND hwnd) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;

	piSmtc = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControls*)GetWindowUserData(hwnd);

	mciSendStringA("pause MySound1", NULL, 0, NULL);
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_PlaybackStatus(piSmtc, MediaPlaybackStatus_Paused);
}

void WndProc_User_Stop(HWND hwnd) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;

	piSmtc = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControls*)GetWindowUserData(hwnd);

	mciSendStringA("stop MySound1", NULL, 0, NULL);
	mciSendStringA("close MySound1", NULL, 0, NULL);
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls_put_PlaybackStatus(piSmtc, MediaPlaybackStatus_Stopped);
}

void WndProc_User_Previous(HWND hwnd) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;

	piSmtc = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControls*)GetWindowUserData(hwnd);

	WndProc_User_Stop(hwnd);
	nCurSong = nSongsCount - ((nSongsCount - nCurSong) % nSongsCount) - 1;
	WndProc_User_Play(hwnd);
}

void WndProc_User_Next(HWND hwnd) {
	__x_ABI_CWindows_CMedia_CISystemMediaTransportControls *piSmtc;

	piSmtc = (__x_ABI_CWindows_CMedia_CISystemMediaTransportControls*)GetWindowUserData(hwnd);

	WndProc_User_Stop(hwnd);
	nCurSong = (nCurSong + 1) % nSongsCount;
	WndProc_User_Play(hwnd);
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
	case WM_USER_Play:
		WndProc_User_Play(hwnd);
		break;
	case WM_USER_Pause:
		WndProc_User_Pause(hwnd);
		break;
	case WM_USER_Stop:
		WndProc_User_Stop(hwnd);
		break;
	case WM_USER_Previous:
		WndProc_User_Previous(hwnd);
		break;
	case WM_USER_Next:
		WndProc_User_Next(hwnd);
		break;
	}

	return DefWindowProcA(hwnd, msg, wParam, lParam);
}