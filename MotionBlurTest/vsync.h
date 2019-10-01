#pragma once

#include <stdbool.h>

typedef void(*VSyncPaintFunc)(void *pData);

bool WaitForVBlankEx(unsigned nAdapter, unsigned nMonitor);
bool WaitForVBlank(void);
bool PerformVSyncPaint(VSyncPaintFunc pFunc, void *pData);