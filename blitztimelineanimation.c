#include "prism/blitztimelineanimation.h"

#include "prism/datastructures.h"
#include "prism/blitzentity.h"
#include "prism/blitzcamerahandler.h"
#include "prism/log.h"
#include "prism/system.h"

typedef struct {
	int mAnimationNumber;
	Tick mTime;

} ActiveAnimation;

typedef struct {
	int mEntityID;

	BlitzTimelineAnimations* mAnimations;

	List mActiveAnimations;
} BlitzTimelineAnimationEntry;

static struct {
	IntMap mEntries;
} gData;

static void loadBlitzTimelineAnimationHandler(void* tData) {
	(void)tData;
	gData.mEntries = new_int_map();
}

static void interpolateFloatAnimationStep(BlitzTimelineAnimationStep* tStep, double t, int tEntityID, void(*tFunc)(int, double)) {
	double val1 = atof(tStep->mStartValue);
	double val2 = atof(tStep->mEndValue);
	
	double trueVal = val1 + t*(val2 - val1);
	tFunc(tEntityID, trueVal);
}

static void updateSingleActiveAnimationStepTarget(BlitzTimelineAnimationStep* tStep, double t, int tEntityID) {
	if (tStep->mTargetType == BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_POSITION_X) {
		interpolateFloatAnimationStep(tStep, t, tEntityID, setBlitzEntityPositionX);
	} else 	if (tStep->mTargetType == BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_POSITION_Y) {
		interpolateFloatAnimationStep(tStep, t, tEntityID, setBlitzEntityPositionY);
	}
	else {
		logWarningFormat("Unimplemented target type: %d", tStep->mTargetType);
	}
}

typedef struct {
	ActiveAnimation* mActiveAnimation;
	BlitzTimelineAnimationEntry* mEntry;
} BlitzTimelineActiveAnimationStepUpdateCaller;

static void updateSingleActiveAnimationStep(void* tCaller, void* tData) {
	BlitzTimelineActiveAnimationStepUpdateCaller* caller = tCaller;
	BlitzTimelineAnimationStep* step = tData;
	ActiveAnimation* activeAnimation = caller->mActiveAnimation;

	if (activeAnimation->mTime < step->mTime) return;
	Tick endTime = step->mTime + step->mDuration;
	if (activeAnimation->mTime > endTime) return;

	Tick timeOffset = activeAnimation->mTime - step->mTime;
	double t;
	if (step->mDuration == 0) t = 0;
	else t = timeOffset / (double)step->mDuration;

	updateSingleActiveAnimationStepTarget(step, t, caller->mEntry->mEntityID);
}

static int updateSingleActiveAnimation(void* tCaller, void* tData) {
	BlitzTimelineAnimationEntry* entry = tCaller;
	ActiveAnimation* e = tData;
	BlitzTimelineAnimation* animation = getBlitzTimelineAnimation(entry->mAnimations, e->mAnimationNumber);

	if (handleTickDurationAndCheckIfOver(&e->mTime, animation->mDuration)) {
		return 1;
	}

	BlitzTimelineActiveAnimationStepUpdateCaller caller;
	caller.mActiveAnimation = e;
	caller.mEntry = entry;
	vector_map(&animation->BlitzTimelineAnimationSteps, updateSingleActiveAnimationStep, &caller);

	return 0;
}

static void updateSingleBlitzTimelineAnimationEntry(void* tCaller, void* tData) {
	(void)tCaller;
	BlitzTimelineAnimationEntry* e = tData;
	list_remove_predicate(&e->mActiveAnimations, updateSingleActiveAnimation, e);
}

static void updateBlitzTimelineAnimationHandler(void* tData) {
	(void)tData;
	int_map_map(&gData.mEntries, updateSingleBlitzTimelineAnimationEntry, NULL);
}

ActorBlueprint BlitzTimelineAnimationHandler = {
	.mLoad = loadBlitzTimelineAnimationHandler,
	.mUpdate = updateBlitzTimelineAnimationHandler,
};

static void unregisterEntity(int tEntityID);

static BlitzComponent BlitzTimelineAnimationComponent = {
	.mUnregisterEntity = unregisterEntity,
};

static BlitzTimelineAnimationEntry* getBlitzTimelineAnimationEntry(int tEntityID) {
	if (!int_map_contains(&gData.mEntries, tEntityID)) {
		logErrorFormat("Entity with ID %d does not have a timeline animation component.", tEntityID);
		abortSystem();
	}

	return int_map_get(&gData.mEntries, tEntityID);
}

void addBlitzTimelineComponent(int tEntityID, BlitzTimelineAnimations* tAnimations) {
	BlitzTimelineAnimationEntry* e = allocMemory(sizeof(BlitzTimelineAnimationEntry));
	e->mEntityID = tEntityID;
	e->mAnimations = tAnimations;
	e->mActiveAnimations = new_list();

	registerBlitzComponent(tEntityID, BlitzTimelineAnimationComponent);
	int_map_push_owned(&gData.mEntries, tEntityID, e);
}

static void unregisterEntity(int tEntityID) {
	BlitzTimelineAnimationEntry* e = getBlitzTimelineAnimationEntry(tEntityID);
	delete_list(&e->mActiveAnimations);
	int_map_remove(&gData.mEntries, tEntityID);
}

void playBlitzTimelineAnimation(int tEntityID, int tAnimation) {
	BlitzTimelineAnimationEntry* entry = getBlitzTimelineAnimationEntry(tEntityID);
	
	ActiveAnimation* e = allocMemory(sizeof(ActiveAnimation));
	e->mAnimationNumber = tAnimation;
	e->mTime = 0;

	list_push_back_owned(&entry->mActiveAnimations, e);
}