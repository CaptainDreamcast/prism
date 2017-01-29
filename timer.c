#include "include/timer.h"

typedef struct TimerElement_internal{

	int mID;
	Duration mNow;
	Duration mDuration;
	TimerCB mCB;
	void* mCaller;

	struct TimerElement_internal* mNext;

} TimerElement;

typedef struct {

	TimerElement* mFirst;
	int mSize;

} TimerList;

static struct {
	int gIDs;
	TimerList mList;
} gData;

int addTimerCB(Duration tDuration, TimerCB tCB, void* tCaller){
	TimerElement* e = malloc(sizeof(TimerElement));
	e->mID = gData.gIDs++;
	e->mNow = 0;
	e->mDuration = tDuration;
	e->mCB = tCB;
	e->mCaller = tCaller;
	
	TimerElement* next = gData.mList.mFirst;
	gData.mList.mFirst = e;
	e->mNext = next;
	gData.mList.mSize++;

	return e->mID;
}

void setupTimer(){
	gData.gIDs = 0;
	gData.mList.mSize = 0;
	gData.mList.mFirst = NULL;
}

static void eraseTimerElement(TimerElement* e){
		if(gData.mList.mFirst == e) {
			gData.mList.mFirst = e->mNext;
		}
		free(e);
		gData.mList.mSize--;
		
}

void updateTimer(){
	int left = gData.mList.mSize;
	TimerElement* cur = gData.mList.mFirst;
	while(left--){
		TimerElement* next = cur->mNext;	
		int isOver = handleDurationAndCheckIfOver(&cur->mNow, cur->mDuration);
		if(isOver) {
			cur->mCB(cur->mCaller);
			eraseTimerElement(cur);
		}
		cur = next;
	}
}

void clearTimer(){
	int left = gData.mList.mSize;
	TimerElement* cur = gData.mList.mFirst;
	while(left--){
		TimerElement* next = cur->mNext;		
		eraseTimerElement(cur);
		cur = next;
	}
}

void shutdownTimer(){
	clearTimer();
}
