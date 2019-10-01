#include "d3dhelper.h"

#ifdef _MSC_VER
#define restrict __restrict
#endif

bool InitD3DData_Inner(HWND hwnd, D3DDeviceData *restrict pd3ddd) {
	D3DPRESENT_PARAMETERS d3dpp;
	HRESULT hResult;

	pd3ddd->pid3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (pd3ddd->pid3d9 == NULL)
		return false;

	d3dpp = (D3DPRESENT_PARAMETERS) { 0 };
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	hResult = IDirect3D9_CreateDevice(
		pd3ddd->pid3d9,
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&pd3ddd->pid3d9Device
	);
	if (hResult != D3D_OK) {
		IDirect3D9_Release(pd3ddd->pid3d9);
		return false;
	}
	IDirect3DDevice9_GetBackBuffer(pd3ddd->pid3d9Device, 0, 0, D3DBACKBUFFER_TYPE_MONO, &pd3ddd->pid3d9Surface);
	if (hResult != D3D_OK) {
		IDirect3DDevice9_Release(pd3ddd->pid3d9Device);
		IDirect3D9_Release(pd3ddd->pid3d9);
		return false;
	}

	return true;
}

pD3DDeviceData InitD3DData(HWND hwnd) {
	pD3DDeviceData pd3ddd;

	pd3ddd = (pD3DDeviceData)malloc(sizeof(D3DDeviceData));
	if (pd3ddd == NULL)
		return NULL;

	if (!InitD3DData_Inner(hwnd, pd3ddd)) {
		free(pd3ddd);
		return NULL;
	}

	return pd3ddd;
}

bool UninitD3DData(pD3DDeviceData pd3ddd) {
	if (pd3ddd == NULL)
		return false;
	IDirect3DSurface9_Release(pd3ddd->pid3d9Surface);
	IDirect3DDevice9_Release(pd3ddd->pid3d9Device);
	IDirect3D9_Release(pd3ddd->pid3d9);
	free(pd3ddd);
	return true;
}

bool ResetD3DData(pD3DDeviceData pd3ddd, bool bFullReset) {
	D3DDEVICE_CREATION_PARAMETERS d3ddcp;
	D3DPRESENT_PARAMETERS d3dpp;
	HRESULT hResult, hResult2;

	if (pd3ddd == NULL)
		return false;

	if (bFullReset) {
		if (IDirect3DDevice9_GetCreationParameters(pd3ddd->pid3d9Device, &d3ddcp) != D3D_OK) {
			UninitD3DData(pd3ddd);
			return false;
		}
		IDirect3DSurface9_Release(pd3ddd->pid3d9Surface);
		IDirect3DDevice9_Release(pd3ddd->pid3d9Device);
		IDirect3D9_Release(pd3ddd->pid3d9);
		if (!InitD3DData_Inner(d3ddcp.hFocusWindow, pd3ddd)) {
			free(pd3ddd);
			return false;
		}
	}
	else {
		d3dpp = (D3DPRESENT_PARAMETERS) { 0 };
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3dpp.Windowed = true;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

		IDirect3DSurface9_Release(pd3ddd->pid3d9Surface);
		hResult = IDirect3DDevice9_Reset(pd3ddd->pid3d9Device, &d3dpp);
		hResult2 = IDirect3DDevice9_GetBackBuffer(pd3ddd->pid3d9Device, 0, 0, D3DBACKBUFFER_TYPE_MONO, &pd3ddd->pid3d9Surface);
		if (hResult != D3D_OK || hResult2 != D3D_OK) {
			IDirect3DDevice9_Release(pd3ddd->pid3d9Device);
			IDirect3D9_Release(pd3ddd->pid3d9);
			free(pd3ddd);
			return false;
		}
	}

	return true;
}

bool D3DDevice_IsDeviceOK(pD3DDeviceData pd3ddd) {
	return pd3ddd != NULL && IDirect3DDevice9_TestCooperativeLevel(pd3ddd->pid3d9Device) == D3D_OK;
}

bool D3DBackBuffer_LockRect(pD3DDeviceData pd3ddd, RECT *prtLock, D3DLOCKED_RECT *pd3dRectLocked) {
	if (pd3ddd == NULL || pd3dRectLocked == NULL)
		return false;
	return IDirect3DSurface9_LockRect(pd3ddd->pid3d9Surface, pd3dRectLocked, prtLock, D3DLOCK_DISCARD) == D3D_OK;
}

bool D3DBackBuffer_UnlockRect(pD3DDeviceData pd3ddd) {
	if (pd3ddd == NULL)
		return false;
	return IDirect3DSurface9_UnlockRect(pd3ddd->pid3d9Surface) == D3D_OK;
}

bool D3DDevice_Present(pD3DDeviceData pd3ddd) {
	if (pd3ddd == NULL)
		return false;
	return IDirect3DDevice9_Present(pd3ddd->pid3d9Device, NULL, NULL, NULL, NULL) == D3D_OK;
}