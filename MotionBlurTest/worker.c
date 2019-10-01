#include "worker.h"

#include <Windows.h>
#include <process.h>

typedef struct tagWorker {
	WorkerState nState;
	WorkerFunc funcWork;
	void *pUserData;
	int nLastWorkReturn;
} Worker;

void WorkerWrapperThread(void *pData) {
	pWorker pWk;

	pWk = (pWorker)pData;

	pWk->nState = WORKER_RUNNING;
	pWk->nLastWorkReturn = pWk->funcWork(pWk, pWk->pUserData);
	pWk->nState = WORKER_STOPPED;
}

pWorker CreateWorker(void) {
	pWorker pWk;

	pWk = (pWorker)malloc(sizeof(Worker));
	if (pWk == NULL)
		return NULL;

	pWk->nState = WORKER_STOPPED;
	pWk->funcWork = NULL;
	pWk->pUserData = NULL;
	pWk->nLastWorkReturn = 0;

	return pWk;
}

bool DestroyWorker(pWorker pWk) {
	if (pWk == NULL)
		return false;
	if (!(pWk->nState & WORKER_STOPPED))
		return false;
	free(pWk);
	return true;
}

WorkerState QueryWorkerState(pWorker pWk) {
	return pWk == NULL ? (WorkerState)-1 : pWk->nState;
}

WorkerState ModifyWorkerState(pWorker pWk, WorkerState nNewState) {
	if (pWk == NULL)
		return (WorkerState)-1;
	return InterlockedExchange(&pWk->nState, nNewState);
}

bool SetWorkerWork(pWorker pWk, WorkerFunc funcWork) {
	if (pWk == NULL)
		return false;
	if (!(pWk->nState & WORKER_STOPPED))
		return false;
	pWk->funcWork = funcWork;
	return false;
}

bool StartWorkerWork(pWorker pWk, void *pData, uint32_t nWorkMethod) {
	if (pWk == NULL)
		return false;
	if (!(pWk->nState & WORKER_STOPPED))
		return false;

	pWk->pUserData = pData;
	switch (nWorkMethod) {
	case WORKER_WORK_SYNC:
		WorkerWrapperThread(pWk);
		break;
	case WORKER_WORK_ASYNC:
		_beginthread(WorkerWrapperThread, 0, pWk);
		while (!(((volatile Worker*)(pWk))->nState & WORKER_RUNNING));
		break;
	default:
		return false;
	}

	return true;
}

bool StopWorkerWork(pWorker pWk, bool bWait) {
	if (pWk == NULL)
		return false;
	if (pWk->nState & WORKER_STOPPED)
		return false;
	pWk->nState = WORKER_STOPPING;
	if (bWait)
		while (!(((volatile Worker*)(pWk))->nState & WORKER_STOPPED));
	return true;
}

int GetWorkerLastWorkReturn(pWorker pWk) {
	if (pWk == NULL)
		return -1;
	return pWk->nLastWorkReturn;
}