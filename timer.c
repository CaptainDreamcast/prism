#include "prism/timer.h"

#include "prism/memoryhandler.h"
#include "prism/datastructures.h"

typedef struct TimerElement_internal{
	Duration mNow;
	Duration mDuration;
	TimerCB mCB;
	void* mCaller;

} TimerElement;

static struct {
	List mList;
} gData;

int addTimerCB(Duration tDuration, TimerCB tCB, void* tCaller){
	TimerElement* e = allocMemory(sizeof(TimerElement));
	e->mNow = 0;
	e->mDuration = tDuration;
	e->mCB = tCB;
	e->mCaller = tCaller;

	return list_push_front_owned(&gData.mList, (void*)e);
}

void removeTimer(int tID)
{
	list_remove(&gData.mList, tID);
}

void setupTimer(){
	gData.mList = new_list();
}

static int updateCB(void* tCaller, void* tData) {
	(void) tCaller;
	TimerElement* cur = tData;
	int isOver = handleDurationAndCheckIfOver(&cur->mNow, cur->mDuration);

	if(isOver) {
		cur->mCB(cur->mCaller);
		return 1;
	}

	return 0;
}

void updateTimer(){
	list_remove_predicate(&gData.mList, updateCB, NULL);
}

void clearTimer(){
	list_empty(&gData.mList);
}

void shutdownTimer(){
	clearTimer();
}
