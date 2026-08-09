#include "prism/thread.h"

#include <thread>
#include <mutex>
#include <condition_variable>

namespace prism {

	struct LinuxSemaphore {
		std::mutex mMutex;
		std::condition_variable mCondition;
		int mCount;
	};

	void initThreading() {}
	void shutdownThreading() {}

	int startThread(void(tFunc)(void*), void* tCaller)
	{
		std::thread t(tFunc, tCaller);
		t.detach();
		return 1;
	}

	Semaphore createSemaphore(int tInitialAccessesAllowed)
	{
		auto* sem = new LinuxSemaphore();
		sem->mCount = tInitialAccessesAllowed;
		return (Semaphore)sem;
	}

	void destroySemaphore(Semaphore tSemaphore)
	{
		delete (LinuxSemaphore*)tSemaphore;
	}

	void lockSemaphore(Semaphore tSemaphore)
	{
		auto* sem = (LinuxSemaphore*)tSemaphore;
		std::unique_lock<std::mutex> lock(sem->mMutex);
		sem->mCondition.wait(lock, [sem] { return sem->mCount > 0; });
		sem->mCount--;
	}

	void releaseSemaphore(Semaphore tSemaphore)
	{
		auto* sem = (LinuxSemaphore*)tSemaphore;
		{
			std::lock_guard<std::mutex> lock(sem->mMutex);
			sem->mCount++;
		}
		sem->mCondition.notify_one();
	}

	void terminateSelfAsThread(int /*tReturnValue*/) {}

	void imguiThread() {}

}
