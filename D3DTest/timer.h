#pragma once

#include <stdbool.h>

typedef struct tagHRTimer *pHRTimer;

pHRTimer CreateHighResolutionTimer(void);
bool DestroyHighResolutionTimer(pHRTimer pTimer);
bool BeginHighResolutionTimer(pHRTimer pTimer);
bool PauseHighResolutionTimer(pHRTimer pTimer);
bool StopHighResolutionTimer(pHRTimer pTimer);
double fGetHighResolutionTimerValue(pHRTimer pTimer);	//Unit: seconds