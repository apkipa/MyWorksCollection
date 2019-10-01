#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t WorkerState;
typedef struct tagWorker *pWorker;
typedef int(*WorkerFunc)(pWorker pWk, void *pData);

#define WORKER_STOPPED 1
#define WORKER_STOPPING (1 << 8)
#define WORKER_RUNNING 2

#define WORKER_WORK_SYNC 1
#define WORKER_WORK_ASYNC 2

#define IsWorkerStopped(pWk) ((bool)(QueryWorkerState(pWk) & WORKER_STOPPED))
#define IsWorkerStopping(pWk) ((bool)(QueryWorkerState(pWk) & WORKER_STOPPING))
#define IsWorkerRunning(pWk) ((bool)(QueryWorkerState(pWk) & WORKER_RUNNING))

pWorker CreateWorker(void);
bool DestroyWorker(pWorker pWk);
WorkerState QueryWorkerState(pWorker pWk);
bool SetWorkerWork(pWorker pWk, WorkerFunc funcWork);
bool StartWorkerWork(pWorker pWk, void *pData, uint32_t nWorkMethod);
bool StopWorkerWork(pWorker pWk, bool bWait);
int GetWorkerLastWorkReturn(pWorker pWk);