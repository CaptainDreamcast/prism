#include "prism/tweening.h"

#include <stdio.h>

#include <prism/datastructures.h>
#include <prism/memoryhandler.h>
#include <prism/math.h>
#include <prism/stlutil.h>

using namespace std;

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
	map<int, Tween> mTweens;
	int mIsActive;
} gTweening;

void setupTweening()
{
	if (gTweening.mIsActive) {
		shutdownTweening();
	}
	gTweening.mTweens.clear();
	gTweening.mIsActive = 1;
}

void shutdownTweening()
{

	gTweening.mTweens.clear();
	gTweening.mIsActive = 0;
}

static int updateTween(void* tCaller,Tween& tData) {
	(void)tCaller;
	Tween* e = &tData;

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
	stl_int_map_remove_predicate(gTweening.mTweens, updateTween);
}

int tweenDouble(double * tDst, double tStart, double tEnd, TweeningFunction tFunc, Duration tDuration, TweeningCBFunction tCB, void * tCaller)
{
	Tween e;
	e.mDst = tDst;
	e.mStart = tStart;
	e.mEnd = tEnd;
	e.mFunc = tFunc;
	e.mNow = 0;
	e.mDuration = tDuration;
	e.mCB = tCB;
	e.mCaller = tCaller;

	*tDst = tStart;

	return stl_int_map_push_back(gTweening.mTweens, e);
}

void removeTween(int tID)
{
	gTweening.mTweens.erase(tID);
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

