#define COBJMACROS
#define INITGUID

#include "vsync.h"

#include <Windows.h>
#include <dwmapi.h>
#include <dxgi.h>

#pragma comment(lib, "dwmapi")
#pragma comment(lib, "dxgi")

#define ReleaseInterface(pI) IUnknown_Release((IUnknown*)(pI))

bool IsDwmEnabled(void) {
	BOOL b = FALSE;
	DwmIsCompositionEnabled(&b);
	return (bool)b;
}

IDXGIOutput* GetIDXGIOutput(unsigned nAdapter, unsigned nOutput) {
	IDXGIFactory *pdxgiFactory;
	IDXGIAdapter *pdxgiAdapter;
	IDXGIOutput *pdxgiOutput;

	if (CreateDXGIFactory(&IID_IDXGIFactory, &pdxgiFactory) != S_OK)
		return NULL;
	if (IDXGIFactory_EnumAdapters(pdxgiFactory, nAdapter, &pdxgiAdapter) != S_OK) {
		IDXGIFactory_Release(pdxgiFactory);
		return NULL;
	}
	if (IDXGIAdapter_EnumOutputs(pdxgiAdapter, nOutput, &pdxgiOutput) != S_OK) {
		IDXGIFactory_Release(pdxgiFactory);
		IDXGIAdapter_Release(pdxgiAdapter);
		return NULL;
	}

	IDXGIAdapter_Release(pdxgiAdapter);
	IDXGIFactory_Release(pdxgiFactory);

	return pdxgiOutput;
}

IDXGIOutput* GetIDXGIDefaultOutput(void) {
	return GetIDXGIOutput(0, 0);
}

bool WaitForVBlankEx(unsigned nAdapter, unsigned nMonitor) {
	IDXGIOutput *pdxgiOutput;
	bool bSucceeded;
	pdxgiOutput = GetIDXGIOutput(nAdapter, nMonitor);
	if (pdxgiOutput == NULL)
		return false;
	bSucceeded = SUCCEEDED(IDXGIOutput_WaitForVBlank(pdxgiOutput));	//This is always true, why?
	IDXGIOutput_Release(pdxgiOutput);
	return bSucceeded;
}

bool WaitForVBlank(void) {
	return WaitForVBlankEx(0, 0);
}

bool PerformVSyncPaint(VSyncPaintFunc pFunc, void *pData) {
	bool bTemp;
	if (pFunc == NULL)
		return false;
	/*
	if (IsDwmEnabled()) {
		pFunc(pData);
		return DwmFlush() == S_OK;
	}
	else */{
		//DwmFlush() is good, because it will return after composition is finished, in order to perfectly avoid tearing
		//but DwmFlush() is unreliable, since it will wait until all programs finish their drawings - hard to keep vsync
		bTemp = WaitForVBlank();
		//Sleep(1);
		pFunc(pData);
		return bTemp;
	}
}