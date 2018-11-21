#include "prism/thread.h"

#include <windows.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"

typedef struct {
	int mID;
	DWORD mThreadID;
	HANDLE mThreadHandle;

	void(*mFunc)(void*);
	void* mCaller;
} ThreadData;

static struct {
	IntMap mThreads;
	Semaphore mThreadMapAccessSemaphore;
} gData;

void initThreading() {
	gData.mThreadMapAccessSemaphore = createSemaphore(1);
	gData.mThreads = new_int_map();
}

static int forceShutdownSingleThread(void* tCaller, void* tData) {
	(void)tCaller;
	ThreadData* e = tData;

	TerminateThread(e->mThreadHandle, 0);

	return 1;
}

void shutdownThreading()
{
	lockSemaphore(gData.mThreadMapAccessSemaphore);
	int_map_remove_predicate(&gData.mThreads, forceShutdownSingleThread, NULL);
	delete_int_map(&gData.mThreads);
	releaseSemaphore(&gData.mThreadMapAccessSemaphore);
}

DWORD WINAPI threadFunction(LPVOID lpParam) {
	ThreadData* e = lpParam;

	e->mFunc(e->mCaller);

	CloseHandle(e->mThreadHandle);
	lockSemaphore(gData.mThreadMapAccessSemaphore);
	int_map_remove(&gData.mThreads, e->mID);
	releaseSemaphore(gData.mThreadMapAccessSemaphore);

	return 0;
}

int startThread(void(tFunc)(void *), void* tCaller)
{
	ThreadData* e = allocMemory(sizeof(ThreadData));
	e->mFunc = tFunc;
	e->mCaller = tCaller;

	lockSemaphore(gData.mThreadMapAccessSemaphore);
	e->mID = int_map_push_back_owned(&gData.mThreads, e);
	releaseSemaphore(gData.mThreadMapAccessSemaphore);

	e->mThreadHandle = CreateThread(NULL, 0, threadFunction, e, 0, &e->mThreadID);
	return e->mID;
}

Semaphore createSemaphore(int tInitialAccessesAllowed)
{
	HANDLE ret = CreateSemaphore(NULL, tInitialAccessesAllowed, 1, NULL);
	return ret;
}

void destroySemaphore(Semaphore tSemaphore)
{
	HANDLE sem = tSemaphore;
	CloseHandle(sem);
}

void lockSemaphore(Semaphore tSemaphore)
{
	HANDLE sem = tSemaphore;
	WaitForSingleObject(sem, INFINITE);
}

void releaseSemaphore(Semaphore tSemaphore)
{
	HANDLE sem = tSemaphore;
	ReleaseSemaphore(sem, 1, NULL);
}
