#include "prism/tweening.h"

#include <stdio.h>

#include <prism/datastructures.h>
#include <prism/memoryhandler.h>
#include <prism/math.h>

typedef struct {
	double* mDst;
	TweeningFunction mFunc;
	
	double mStart;
	double mEnd;

	Duration mNow;
	Duration mDuration;

	TweeningCBFunction mCB;
	void* mCaller;
} Tween;

static struct {
	IntMap mTweens;
	int mIsActive;
} gData;

void setupTweening()
{
	if (gData.mIsActive) {
		shutdownTweening();
	}
	gData.mTweens = new_int_map();
	gData.mIsActive = 1;
}

void shutdownTweening()
{

	delete_int_map(&gData.mTweens);
	gData.mIsActive = 0;
}

static int updateTween(void* tCaller, void* tData) {
	(void)tCaller;
	Tween* e = (Tween*)tData;

	int isOver = handleDurationAndCheckIfOver(&e->mNow, e->mDuration);

	double baseT = getDurationPercentage(e->mNow, e->mDuration);
	double t = e->mFunc(baseT);

	*e->mDst = e->mStart + t * (e->mEnd - e->mStart);

	if (isOver && e->mCB) {
		e->mCB(e->mCaller);
	}

	return isOver;
}

void updateTweening()
{
	int_map_remove_predicate(&gData.mTweens, updateTween, NULL);
}

int tweenDouble(double * tDst, double tStart, double tEnd, TweeningFunction tFunc, Duration tDuration, TweeningCBFunction tCB, void * tCaller)
{
	Tween* e = (Tween*)allocMemory(sizeof(Tween));
	e->mDst = tDst;
	e->mStart = tStart;
	e->mEnd = tEnd;
	e->mFunc = tFunc;
	e->mNow = 0;
	e->mDuration = tDuration;
	e->mCB = tCB;
	e->mCaller = tCaller;

	*tDst = tStart;

	return int_map_push_back_owned(&gData.mTweens, e);
}

void removeTween(int tID)
{
	int_map_remove(&gData.mTweens, tID);
}

double linearTweeningFunction(double t) {
	return t;
}

double quadraticTweeningFunction(double t) {
	return t*t;
}

double inverseQuadraticTweeningFunction(double t) {
	return 1-quadraticTweeningFunction(1-t);
}

double squareRootTweeningFunction(double t) {
	return sqrt(t);
}

double overshootTweeningFunction(double t) {
	double overshoot = 1.5;
	if (t < 0.8) return linearTweeningFunction((t / 0.8) * overshoot);
	else return linearTweeningFunction(overshoot + ((t-0.8) / 0.2) * (1.0 - overshoot));
}

double transformAtEndTweeningFunction(double t)
{
	if (t >= 1) return 1;
	else return 0;
}
