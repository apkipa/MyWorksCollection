#pragma once

#include <Windows.h>
#include <stdbool.h>
#include <d3d9.h>

typedef struct tagD3DDeviceData {
	IDirect3D9 *pid3d9;
	IDirect3DDevice9 *pid3d9Device;
	IDirect3DSurface9 *pid3d9Surface;
} D3DDeviceData, *pD3DDeviceData;

pD3DDeviceData InitD3DData(HWND hwnd);
bool UninitD3DData(pD3DDeviceData pd3ddd);
bool ResetD3DData(pD3DDeviceData pd3ddd, bool bFullReset);	//When a reset failure occurs, it frees pd3ddd automatically
bool D3DDevice_IsDeviceOK(pD3DDeviceData pd3ddd);
bool D3DBackBuffer_LockRect(pD3DDeviceData pd3ddd, RECT *prtLock, D3DLOCKED_RECT *pd3dRectLocked);
bool D3DBackBuffer_UnlockRect(pD3DDeviceData pd3ddd);
bool D3DDevice_Present(pD3DDeviceData pd3ddd);