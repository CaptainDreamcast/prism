#include "prism/thread.h"

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"

void initThreading() {
	// TODO
}

void shutdownThreading()
{
	// TODO
}

int startThread(void(tFunc)(void *), void* tCaller)
{
	(void)tFunc;
	(void)tCaller;
	// TODO
	return -1;
}

Semaphore createSemaphore(int tInitialAccessesAllowed)
{
	(void)tInitialAccessesAllowed;
	return NULL; // TODO
}

void destroySemaphore(Semaphore tSemaphore)
{
	(void)tSemaphore;
	// TODO
}

void lockSemaphore(Semaphore tSemaphore)
{
	(void)tSemaphore;
	// TODO
}

void releaseSemaphore(Semaphore tSemaphore)
{
	(void)tSemaphore;
	// TODO
}
