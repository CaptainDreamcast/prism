#include "prism/timer.h"

#include "prism/memoryhandler.h"
#include "prism/datastructures.h"

#include "prism/stlutil.h"

using namespace std;

namespace prism {

	typedef struct TimerElement_internal {
		Duration mNow;
		Duration mDuration;
		TimerCB mCB;
		void* mCaller;

	} TimerElement;

	static struct {
		map<int, TimerElement> mList;
	} gTimerData;

	int addTimerCB(Duration tDuration, TimerCB tCB, void* tCaller) {
		TimerElement e;
		e.mNow = 0;
		e.mDuration = tDuration;
		e.mCB = tCB;
		e.mCaller = tCaller;

		return stl_int_map_push_back(gTimerData.mList, e);
	}

	void removeTimer(int tID)
	{
		gTimerData.mList.erase(tID);
	}

	void setupTimer() {
		gTimerData.mList.clear();
	}

	static int updateCB(void* tCaller, TimerElement& tData) {
		(void)tCaller;
		TimerElement* cur = &tData;
		int isOver = handleDurationAndCheckIfOver(&cur->mNow, cur->mDuration);

		if (isOver) {
			if (cur->mCB)
			{
				cur->mCB(cur->mCaller);
			}
			return 1;
		}

		return 0;
	}

	void updateTimer() {
		stl_int_map_remove_predicate(gTimerData.mList, updateCB);
	}

	void clearTimer() {
		gTimerData.mList.clear();
	}

	void shutdownTimer() {
		clearTimer();
	}

	int hasTimerFinished(int tID) {
		return gTimerData.mList.find(tID) == gTimerData.mList.end();
	}
}