#include "include/stagehandler.h"

#include "include/script.h"
#include "include/datastructures.h"
#include "include/memoryhandler.h"
#include "include/file.h"
#include "include/system.h"
#include "include/log.h"
#include "include/math.h"

static struct {
	List mList;

	int mIsLoadingTexturesDirectly;
} gData;

typedef struct {

	Position mPosition;
	char mPath[100];

	Animation mAnimation;

	int mIsLoaded;
	TextureData mTextureData[3];

	int mAnimationID;
} BackgroundPatchData;

typedef struct {
	double mScrollingFactor;
	double mMaxVelocity;
	PhysicsObject mPhysics;
	double mZ;
	List mPatchList;
	ListIterator mCurrentStartPatch;
	ListIterator mCurrentEndPatch;
} SingleBackgroundData;

void setupStageHandler() {
	gData.mList = new_list();
	gData.mIsLoadingTexturesDirectly = 0;
}

static void emptyAll(void* tCaller, void* tData) {
	(void) tCaller;
	SingleBackgroundData* data = tData;
	list_empty(&data->mPatchList);
}

void shutdownStageHandler() {
	list_map(&gData.mList, emptyAll, NULL);
	list_empty(&gData.mList);
}


static void loadStagePatchIfNecessary(BackgroundPatchData* tData, SingleBackgroundData* tBackgroundData) {
	if(tData->mIsLoaded) return;

	Frame i;
	for(i = 0; i < tData->mAnimation.mFrameAmount; i++) {
		char fPath[100];
		getPathWithNumberAffixedFromAssetPath(fPath, tData->mPath, i);
		tData->mTextureData[i] = loadTexture(fPath);
	}

	tData->mAnimationID = playAnimationLoop(tData->mPosition, tData->mTextureData, tData->mAnimation, makeRectangleFromTexture(tData->mTextureData[0]));
	setAnimationScreenPositionReference(tData->mAnimationID, &tBackgroundData->mPhysics.mPosition);

	tData->mIsLoaded = 1;
}

static int isStagePatchOutOfBounds(BackgroundPatchData* tData, SingleBackgroundData* tBackgroundData) {

	double sl = tBackgroundData->mPhysics.mPosition.x;
	double sr = sl + 640.0;

	double l = tData->mPosition.x;
	double r = l + 640; // TODO: Can only know real size after loading texture! Find a fix.

	int isOut = l > sr || r < sl;
	return isOut;
}

static void unloadStagePatchIfNecessary(BackgroundPatchData* tData) {
	if (!tData->mIsLoaded || gData.mIsLoadingTexturesDirectly) return;
	removeHandledAnimation(tData->mAnimationID);

	printf("Unloading\n");

	Frame i;
	for(i = 0; i < tData->mAnimation.mFrameAmount; i++) {
		unloadTexture(tData->mTextureData[i]);
	}

	tData->mIsLoaded = 0;

}

static void setSingleStagePatches(SingleBackgroundData* data) {
	while (data->mCurrentEndPatch != NULL && !isStagePatchOutOfBounds(list_iterator_get(data->mCurrentEndPatch), data)) {
		loadStagePatchIfNecessary(list_iterator_get(data->mCurrentEndPatch), data);

		if (list_has_next(data->mCurrentEndPatch)) list_iterator_increase(&data->mCurrentEndPatch);
		else data->mCurrentEndPatch = NULL;

	}

	while (data->mCurrentStartPatch != NULL && isStagePatchOutOfBounds(list_iterator_get(data->mCurrentStartPatch), data)) {
		unloadStagePatchIfNecessary(list_iterator_get(data->mCurrentStartPatch));

		if (list_has_next(data->mCurrentStartPatch)) list_iterator_increase(&data->mCurrentStartPatch);
		else data->mCurrentStartPatch = NULL;
	}
}

static void updateSingleStage(void* tCaller, void* tData) {
	(void) tCaller;
	SingleBackgroundData* data = tData;

	setDragCoefficient(makePosition(0.1, 0.1, 0.1));
	setMaxVelocityDouble(data->mMaxVelocity);
	handlePhysics(&data->mPhysics);
	resetMaxVelocity();
	resetDragCoefficient();

	setSingleStagePatches(data);
}

void updateStageHandler() {
	list_map(&gData.mList, updateSingleStage, NULL);
}

void setStageHandlerNoDelayedLoading() {
	gData.mIsLoadingTexturesDirectly = 1;
}

int addScrollingBackground(double tScrollingFactor, double tZ) {
	SingleBackgroundData* data = allocMemory(sizeof(SingleBackgroundData));
	data->mScrollingFactor = tScrollingFactor;
	data->mMaxVelocity = INF;
	resetPhysicsObject(&data->mPhysics);
	data->mPhysics.mPosition = makePosition(0, 0, 0);
	data->mZ = tZ;
	data->mPatchList = new_list();
	data->mCurrentStartPatch = NULL;
	data->mCurrentEndPatch = NULL;

	return list_push_front_owned(&gData.mList, data);
}

int addBackgroundElement(int tBackgroundID, Position tPosition, char* tPath, Animation tAnimation) {
	SingleBackgroundData* data = list_get(&gData.mList, tBackgroundID);
	
	BackgroundPatchData* pData = allocMemory(sizeof(BackgroundPatchData));
	pData->mIsLoaded = 0;
	pData->mPosition = tPosition;
	pData->mPosition.z = data->mZ+list_size(&data->mPatchList)*0.001;
	pData->mAnimation = tAnimation;
	strcpy(pData->mPath, tPath);

	int id = list_push_back_owned(&data->mPatchList, pData);	

	if(data->mCurrentStartPatch == NULL) data->mCurrentStartPatch = data->mCurrentEndPatch = list_iterator_begin(&data->mPatchList);
		
	if (gData.mIsLoadingTexturesDirectly) {
		loadStagePatchIfNecessary(pData, data);
	}

	return id;
}

Position getRealScreenPosition(int tBackgroundID, Position tPos) {
	SingleBackgroundData* data = list_get(&gData.mList, tBackgroundID);
	Position p = vecAdd(tPos, vecScale(data->mPhysics.mPosition, -1));
	p.z = tPos.z;
	return p;
}


typedef struct {

	Acceleration mAccel; 

} BackgroundScrollData;

static BackgroundScrollData newBackgroundScrollData(Acceleration tAccel) {
	BackgroundScrollData ret;
	ret.mAccel = tAccel;
	return ret;
}

static void scrollSingleBackground(void* tCaller, void* tData) {
	SingleBackgroundData* data = tData;
	BackgroundScrollData* sData = tCaller;

	data->mPhysics.mAcceleration = vecAdd(data->mPhysics.mAcceleration, vecScale(sData->mAccel, data->mScrollingFactor));
}

void scrollBackgroundRight(double tAccel) {
	BackgroundScrollData sData = newBackgroundScrollData(makePosition(tAccel, 0, 0));

	list_map(&gData.mList, scrollSingleBackground, &sData);
}

Position* getScrollingBackgroundPositionReference(int tID) {
	SingleBackgroundData* data = list_get(&gData.mList, tID);

	return &data->mPhysics.mPosition;
}


void setScrollingBackgroundPosition(int tID, Position tPos) {
	SingleBackgroundData* data = list_get(&gData.mList, tID);
	data->mPhysics.mPosition = tPos;
	data->mCurrentStartPatch = data->mCurrentEndPatch = list_iterator_begin(&data->mPatchList);
	setSingleStagePatches(data);
}

void setScrollingBackgroundMaxVelocity(int tID, double tVel) {
	SingleBackgroundData* data = list_get(&gData.mList, tID);
	data->mMaxVelocity = tVel;
}

PhysicsObject* getScrollingBackgroundPhysics(int tID) {
	SingleBackgroundData* data = list_get(&gData.mList, tID);
	return &data->mPhysics;
}

void setScrollingBackgroundPhysics(int tID, PhysicsObject tPhysics) {
	SingleBackgroundData* data = list_get(&gData.mList, tID);
	data->mPhysics = tPhysics;
}

typedef struct {
	double mZ;
	double mScrollingFactor;
	double mMaxVelocity;
	int mID;
} StageScriptLayerData;

typedef struct {
	Position mPosition;
	char mPath[1024];
	Animation mAnimation;

} StageScriptLayerElementData;

static ScriptPosition loadStageScriptLayerElement(void* tCaller, ScriptPosition tPos) {
	StageScriptLayerElementData* e = tCaller;
	char word[1024];

	tPos = getNextScriptString(tPos, word);

	if (!strcmp("POSITION", word)) {
		tPos = getNextScriptDouble(tPos, &e->mPosition.x);
		tPos = getNextScriptDouble(tPos, &e->mPosition.y);
		e->mPosition.z = 0;
	}
	else if (!strcmp("PATH", word)) {
		tPos = getNextScriptString(tPos, e->mPath);
	}
	else if (!strcmp("ANIMATION", word)) {
		e->mAnimation = createEmptyAnimation();

		int v;
		tPos = getNextScriptInteger(tPos, &v);
		e->mAnimation.mDuration = v;

		tPos = getNextScriptInteger(tPos, &v);
		e->mAnimation.mFrameAmount = v;
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

static ScriptPosition loadStageScriptLayer(void* tCaller, ScriptPosition tPos) {
	StageScriptLayerData* e = tCaller;
	char word[1024];

	tPos = getNextScriptString(tPos, word);

	if (!strcmp("POSITION_Z", word)) {
		tPos = getNextScriptDouble(tPos, &e->mZ);
	}
	else if (!strcmp("SCROLLING_FACTOR", word)) {
		tPos = getNextScriptDouble(tPos, &e->mScrollingFactor);
	}
	else if (!strcmp("MAX_VELOCITY", word)) {
		tPos = getNextScriptDouble(tPos, &e->mMaxVelocity);
	}
	else if (!strcmp("CREATE", word)) {
		e->mID = addScrollingBackground(e->mScrollingFactor, e->mZ);
		setScrollingBackgroundMaxVelocity(e->mID, e->mMaxVelocity);
	}
	else if (!strcmp("ELEMENT", word)) {
		ScriptRegion reg = getScriptRegionAtPosition(tPos);
		StageScriptLayerElementData caller;
		memset(&caller, 0, sizeof(StageScriptLayerElementData));
		executeOnScriptRegion(reg, loadStageScriptLayerElement, &caller);
		tPos = getPositionAfterScriptRegion(tPos.mRegion, reg);
		addBackgroundElement(e->mID, caller.mPosition, caller.mPath, caller.mAnimation);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

static ScriptPosition loadStageScriptStage(void* tCaller, ScriptPosition tPos) {
	(void)tCaller;
	char word[1024];

	tPos = getNextScriptString(tPos, word);

	if (!strcmp("LAYER", word)) {
		ScriptRegion reg = getScriptRegionAtPosition(tPos);
		StageScriptLayerData caller;
		memset(&caller, 0, sizeof(StageScriptLayerData));
		caller.mMaxVelocity = INF;
		executeOnScriptRegion(reg, loadStageScriptLayer, &caller);
		tPos = getPositionAfterScriptRegion(tPos.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

void loadStageFromScript(char* tPath) {
	Script s = loadScript(tPath);
	ScriptRegion reg = getScriptRegion(s, "STAGE");

	executeOnScriptRegion(reg, loadStageScriptStage, NULL);
}