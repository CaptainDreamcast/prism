#include "prism/thread.h"

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"

namespace prism {

    typedef struct {
        int mID;
        pthread_t mThreadID;
        void(*mFunc)(void*);
        void* mCaller;
    } ThreadData;

    static struct {
        IntMap mThreads;
        sem_t mThreadMapAccessSemaphore;
    } gPrismLinuxThreadData;

    void initThreading() {
        sem_init(&gPrismLinuxThreadData.mThreadMapAccessSemaphore, 0, 1);
        gPrismLinuxThreadData.mThreads = new_int_map();
    }

    static int forceShutdownSingleThread(void* tCaller, void* tData) {
        (void)tCaller;
        ThreadData* e = (ThreadData*)tData;

        pthread_cancel(e->mThreadID);

        return 1;
    }

    void shutdownThreading()
    {
        sem_wait(&gPrismLinuxThreadData.mThreadMapAccessSemaphore);
        int_map_remove_predicate(&gPrismLinuxThreadData.mThreads, forceShutdownSingleThread, NULL);
        delete_int_map(&gPrismLinuxThreadData.mThreads);
        sem_post(&gPrismLinuxThreadData.mThreadMapAccessSemaphore);
        sem_destroy(&gPrismLinuxThreadData.mThreadMapAccessSemaphore);
    }

    void* threadFunction(void* lpParam) {
        ThreadData* e = (ThreadData*)lpParam;

        e->mFunc(e->mCaller);

        sem_wait(&gPrismLinuxThreadData.mThreadMapAccessSemaphore);
        int_map_remove(&gPrismLinuxThreadData.mThreads, e->mID);
        sem_post(&gPrismLinuxThreadData.mThreadMapAccessSemaphore);

        return NULL;
    }

    int startThread(void(tFunc)(void*), void* tCaller)
    {
        ThreadData* e = (ThreadData*)allocMemory(sizeof(ThreadData));
        e->mFunc = tFunc;
        e->mCaller = tCaller;

        sem_wait(&gPrismLinuxThreadData.mThreadMapAccessSemaphore);
        e->mID = int_map_push_back_owned(&gPrismLinuxThreadData.mThreads, e);
        sem_post(&gPrismLinuxThreadData.mThreadMapAccessSemaphore);

        pthread_create(&e->mThreadID, NULL, threadFunction, e);
        return e->mID;
    }

    sem_t createSemaphore(int tInitialAccessesAllowed)
    {
        sem_t sem;
        sem_init(&sem, 0, tInitialAccessesAllowed);
        return sem;
    }

    void destroySemaphore(sem_t tSemaphore)
    {
        sem_destroy(&tSemaphore);
    }

    void lockSemaphore(sem_t tSemaphore)
    {
        sem_wait(&tSemaphore);
    }

    void releaseSemaphore(sem_t tSemaphore)
    {
        sem_post(&tSemaphore);
    }

    void terminateSelfAsThread(int /*tReturnValue*/) {
        pthread_exit(NULL);
    }
}