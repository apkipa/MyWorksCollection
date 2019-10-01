#include "timer.h"

#include <Windows.h>
#include <time.h>

//Perform safe operation n1 - n2
#define LargeInteger_SafeSubtract(n1, n2) ((n1) < (n2) ? ~0ULL - ((n2) - (n1) - 1) : (n1) - (n2))

#ifdef _MSC_VER
#define restrict __restrict
#endif

typedef struct tagHRTimer {
	LARGE_INTEGER c1, c2;
	LARGE_INTEGER freq;
	time_t t1, t2;
	double fPast;
	enum {
		HRTimerState_Timing = 1, HRTimerState_Paused, HRTimerState_Stopped
	} nState;
} HRTimer;

pHRTimer CreateHighResolutionTimer(void) {
	pHRTimer pTimer;

	pTimer = (pHRTimer)malloc(sizeof(HRTimer));
	if (pTimer == NULL)
		return NULL;

	QueryPerformanceFrequency(&pTimer->freq);
	pTimer->nState = HRTimerState_Stopped;

	return pTimer;
}

bool DestroyHighResolutionTimer(pHRTimer pTimer) {
	if (pTimer == NULL)
		return false;
	free(pTimer);
	return true;
}

bool BeginHighResolutionTimer(pHRTimer pTimer) {
	if (pTimer == NULL || pTimer->nState == HRTimerState_Timing)
		return false;

	if (pTimer->nState == HRTimerState_Stopped)
		pTimer->fPast = 0;

	pTimer->nState = HRTimerState_Timing;

	QueryPerformanceCounter(&pTimer->c1);
	pTimer->t1 = time(NULL);

	return true;
}

void CalculateHighResolutionTimerValue(HRTimer *restrict pTimer) {
	double fTime;

	fTime = difftime(pTimer->t2, pTimer->t1);
	for (fTime = difftime(pTimer->t2, pTimer->t1); fTime > 5; fTime--) {
		pTimer->c2.QuadPart = LargeInteger_SafeSubtract(pTimer->c2.QuadPart, pTimer->freq.QuadPart);
		pTimer->fPast++;
	}

	pTimer->fPast += (double)LargeInteger_SafeSubtract(pTimer->c2.QuadPart, pTimer->c1.QuadPart) / pTimer->freq.QuadPart;
}

bool PauseHighResolutionTimer(pHRTimer pTimer) {
	if (pTimer == NULL || pTimer->nState == HRTimerState_Paused || pTimer->nState == HRTimerState_Stopped)
		return false;
	QueryPerformanceCounter(&pTimer->c2);
	pTimer->t2 = time(NULL);
	CalculateHighResolutionTimerValue(pTimer);
	pTimer->nState = HRTimerState_Paused;
	return true;
}

bool StopHighResolutionTimer(pHRTimer pTimer) {
	if (pTimer == NULL || pTimer->nState == HRTimerState_Stopped)
		return false;
	if (pTimer->nState != HRTimerState_Paused) {
		QueryPerformanceCounter(&pTimer->c2);
		pTimer->t2 = time(NULL);
		CalculateHighResolutionTimerValue(pTimer);
	}
	pTimer->nState = HRTimerState_Stopped;
	return true;
}

double fGetHighResolutionTimerValue(pHRTimer pTimer) {
	if (PauseHighResolutionTimer(pTimer))
		BeginHighResolutionTimer(pTimer);
	return pTimer->fPast;
}