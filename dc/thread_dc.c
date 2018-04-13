#include "prism/thread.h"

#include <kos.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"

typedef struct {
	int mID;

	void(*mFunc)(void*);
	void* mCaller;

	kthread_t* mThreadHandle;
} ThreadData;

static struct {
	IntMap mThreads;
	Semaphore mThreadMapAccessSemaphore;
} gData;

void initThreading() {
	gData.mThreadMapAccessSemaphore = createSemaphore(1);
	gData.mThreads = new_int_map();
}

void shutdownThreading()
{
	// TODO
}

void* threadFunction(void* tCaller) {
	ThreadData* e = tCaller;

	e->mFunc(e->mCaller);

	lockSemaphore(gData.mThreadMapAccessSemaphore);
	int_map_remove(&gData.mThreads, e->mID);
	releaseSemaphore(gData.mThreadMapAccessSemaphore);

	return NULL;
}

int startThread(void(tFunc)(void *), void* tCaller)
{
	ThreadData* e = allocMemory(sizeof(ThreadData));
	e->mFunc = tFunc;
	e->mCaller = tCaller;

	lockSemaphore(gData.mThreadMapAccessSemaphore);
	e->mID = int_map_push_back_owned(&gData.mThreads, e);
	releaseSemaphore(gData.mThreadMapAccessSemaphore);

	kthread_attr_t attributes = { 1, THD_STACK_SIZE*20, NULL, 5, "" }; // TODO: fix stack size

	e->mThreadHandle = thd_create_ex(&attributes, threadFunction, e);

	return e->mID;
}

Semaphore createSemaphore(int tInitialAccessesAllowed)
{
	semaphore_t* ret = malloc(sizeof(semaphore_t)); // TODO
	ret->count = 0; // TODO: propose KOS fix
	sem_init(ret, tInitialAccessesAllowed);
	return ret;
}

void destroySemaphore(Semaphore tSemaphore)
{
	semaphore_t* sem = tSemaphore; 
	sem_destroy(sem);
	free(sem);
}

void lockSemaphore(Semaphore tSemaphore)
{
	semaphore_t* sem = tSemaphore; 
	sem_wait(sem);
}

void releaseSemaphore(Semaphore tSemaphore)
{
	semaphore_t* sem = tSemaphore; 
	sem_signal(sem);
}
