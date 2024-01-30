#include "prism/thread.h"

#include <Windows.h>

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
} gPrismWindowsThreadData;

void initThreading() {
	gPrismWindowsThreadData.mThreadMapAccessSemaphore = createSemaphore(1);
	gPrismWindowsThreadData.mThreads = new_int_map();
}

static int forceShutdownSingleThread(void* tCaller, void* tData) {
	(void)tCaller;
	ThreadData* e = (ThreadData*)tData;

	TerminateThread(e->mThreadHandle, 0);

	return 1;
}

void shutdownThreading()
{
	lockSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
	int_map_remove_predicate(&gPrismWindowsThreadData.mThreads, forceShutdownSingleThread, NULL);
	delete_int_map(&gPrismWindowsThreadData.mThreads);
	releaseSemaphore(&gPrismWindowsThreadData.mThreadMapAccessSemaphore);
}

DWORD WINAPI threadFunction(LPVOID lpParam) {
	ThreadData* e = (ThreadData*)lpParam;

	e->mFunc(e->mCaller);

	CloseHandle(e->mThreadHandle);
	lockSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
	int_map_remove(&gPrismWindowsThreadData.mThreads, e->mID);
	releaseSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);

	return 0;
}

int startThread(void(tFunc)(void *), void* tCaller)
{
	ThreadData* e = (ThreadData*)allocMemory(sizeof(ThreadData));
	e->mFunc = tFunc;
	e->mCaller = tCaller;

	lockSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);
	e->mID = int_map_push_back_owned(&gPrismWindowsThreadData.mThreads, e);
	releaseSemaphore(gPrismWindowsThreadData.mThreadMapAccessSemaphore);

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

void terminateSelfAsThread(int /*tReturnValue*/) {
	std::terminate();
}