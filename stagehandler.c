#include "include/stagehandler.h"

#include "include/datastructures.h"
#include "include/memoryhandler.h"
#include "include/file.h"


static struct {
	List mList;
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
	PhysicsObject mPhysics;
	List mPatchList;
	ListIterator mCurrentStartPatch;
	ListIterator mCurrentEndPatch;
} SingleBackgroundData;

void setupStageHandler() {
	gData.mList = new_list();
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
	if(!tData->mIsLoaded) return;
	removeHandledAnimation(tData->mAnimationID);

	Frame i;
	for(i = 0; i < tData->mAnimation.mFrameAmount; i++) {
		unloadTexture(tData->mTextureData[i]);
	}

	tData->mIsLoaded = 0;
}

static void updateSingleStage(void* tCaller, void* tData) {
	(void) tCaller;
	SingleBackgroundData* data = tData;
 
	handlePhysics(&data->mPhysics);

	while(list_has_next(data->mCurrentEndPatch) && !isStagePatchOutOfBounds(list_iterator_get(data->mCurrentEndPatch), data)) {
		loadStagePatchIfNecessary(list_iterator_get(data->mCurrentEndPatch), data);
		list_iterator_increase(data->mCurrentEndPatch);
	}


	while(list_has_next(data->mCurrentStartPatch) && isStagePatchOutOfBounds(list_iterator_get(data->mCurrentStartPatch), data)) {
		unloadStagePatchIfNecessary(list_iterator_get(data->mCurrentStartPatch));
		list_iterator_increase(data->mCurrentStartPatch);
	}
}

void updateStageHandler() {
	list_map(&gData.mList, updateSingleStage, NULL);
}

int addScrollingBackground(double tScrollingFactor, double tZ) {
	SingleBackgroundData* data = allocMemory(sizeof(SingleBackgroundData));
	data->mScrollingFactor = tScrollingFactor;
	resetPhysicsObject(&data->mPhysics);
	data->mPhysics.mPosition = makePosition(0, 0, tZ);
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
	pData->mAnimation = tAnimation;
	strcpy(pData->mPath, tPath);
	int id = list_push_back_owned(&data->mPatchList, pData);	

	if(data->mCurrentStartPatch == NULL) data->mCurrentStartPatch = data->mCurrentEndPatch = list_iterator_begin(&data->mPatchList);
		
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
