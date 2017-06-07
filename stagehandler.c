#include "tari/stagehandler.h"

#include "tari/script.h"
#include "tari/datastructures.h"
#include "tari/memoryhandler.h"
#include "tari/file.h"
#include "tari/system.h"
#include "tari/log.h"
#include "tari/math.h"
#include "tari/texturepool.h"




typedef struct {

	Acceleration mAccel;

} BackgroundScrollData;


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
	Position mReferencedPosition;
	Position mTweeningTarget;
	double mZ;
	List mPatchList;
	ListIterator mCurrentStartPatch;
	ListIterator mCurrentEndPatch;
} SingleBackgroundData;

typedef void(*StageHandlerCameraUpdateFunction)(SingleBackgroundData* tData);
typedef void(*StageHandlerCameraAddMovementFunction)(SingleBackgroundData* tData, BackgroundScrollData tScroll);

typedef struct {
	StageHandlerCameraUpdateFunction mUpdate;
	StageHandlerCameraAddMovementFunction mAddMovement;
} StageHandlerCameraStrategy;

typedef struct {
	Position mDirection;
	double mStrength;
} ScreenShake;

static struct {
	List mList;
	StageHandlerCameraStrategy mCamera;

	GeoRectangle mCameraRange;
	int mIsLoadingTexturesDirectly;
	ScreenShake mShake;
} gData;

void setupStageHandler() {
	gData.mList = new_list();
	gData.mIsLoadingTexturesDirectly = 0;
	gData.mCameraRange = makeGeoRectangle(-INF, -INF, INF * 2, INF * 2);
	gData.mShake.mStrength = 0;
	setStageHandlerAccelerationPhysics();
	
}

static void emptyAll(void* tCaller, void* tData) {
	(void)tCaller;
	SingleBackgroundData* data = tData;
	list_empty(&data->mPatchList);
}

void shutdownStageHandler() {
	list_map(&gData.mList, emptyAll, NULL);
	list_empty(&gData.mList);
}


static void loadStagePatchIfNecessary(BackgroundPatchData* tData, SingleBackgroundData* tBackgroundData) {
	if (tData->mIsLoaded) return;

	Frame i;
	for (i = 0; i < tData->mAnimation.mFrameAmount; i++) {
		char fPath[100];
		getPathWithNumberAffixedFromAssetPath(fPath, tData->mPath, i);
		tData->mTextureData[i] = loadTextureFromPool(fPath);
	}

	tData->mAnimationID = playAnimationLoop(tData->mPosition, tData->mTextureData, tData->mAnimation, makeRectangleFromTexture(tData->mTextureData[0]));
	setAnimationScreenPositionReference(tData->mAnimationID, &tBackgroundData->mReferencedPosition);

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

	Frame i;
	for (i = 0; i < tData->mAnimation.mFrameAmount; i++) {
		unloadTextureFromPool(tData->mTextureData[i]);
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

static void clampCameraRange(Vector3D* v, double tScrollingFactor) {
	GeoRectangle rect = scaleGeoRectangleByFactor(gData.mCameraRange, tScrollingFactor);
	*v = clampPositionToGeoRectangle(*v, rect);
}

static void setReferencedPosition(SingleBackgroundData* e) {
	double l = gData.mShake.mStrength*e->mScrollingFactor;
	e->mReferencedPosition = vecAdd(e->mPhysics.mPosition, vecScale(gData.mShake.mDirection, l));
}

static void updateSingleStage(void* tCaller, void* tData) {
	(void)tCaller;
	SingleBackgroundData* data = tData;

	gData.mCamera.mUpdate(data);
	clampCameraRange(&data->mPhysics.mPosition, data->mScrollingFactor);
	setReferencedPosition(data);

	setSingleStagePatches(data);
}

static void updateCameraShake() {
	gData.mShake.mStrength *= 0.9;
	if (gData.mShake.mStrength < 0.5) gData.mShake.mStrength = 0;
	gData.mShake.mStrength = min(gData.mShake.mStrength, 100);

	double angle = randfrom(0, M_PI * 2);
	gData.mShake.mDirection = getDirectionFromAngleZ(angle);
}

void updateStageHandler() {
	updateCameraShake();
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
	data->mTweeningTarget = data->mPhysics.mPosition;
	data->mReferencedPosition = data->mPhysics.mPosition;
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
	pData->mPosition.z = data->mZ + list_size(&data->mPatchList)*0.001;
	pData->mAnimation = tAnimation;
	strcpy(pData->mPath, tPath);

	int id = list_push_back_owned(&data->mPatchList, pData);

	if (data->mCurrentStartPatch == NULL) data->mCurrentStartPatch = data->mCurrentEndPatch = list_iterator_begin(&data->mPatchList);

	if (gData.mIsLoadingTexturesDirectly) {
		loadStagePatchIfNecessary(pData, data);
	}

	return id;
}

TextureData* getBackgroundElementTextureData(int tBackgroundID, int tElementID)
{
	SingleBackgroundData* data = list_get(&gData.mList, tBackgroundID);
	BackgroundPatchData* e = list_get(&data->mPatchList, tElementID);
	return e->mTextureData;
}

Position getRealScreenPosition(int tBackgroundID, Position tPos) {
	SingleBackgroundData* data = list_get(&gData.mList, tBackgroundID);
	Position p = vecAdd(tPos, vecScale(data->mReferencedPosition, -1));
	p.z = tPos.z;
	return p;
}

static BackgroundScrollData newBackgroundScrollData(Acceleration tAccel) {
	BackgroundScrollData ret;
	ret.mAccel = tAccel;
	return ret;
}

static void scrollSingleBackground(void* tCaller, void* tData) {
	SingleBackgroundData* data = tData;
	BackgroundScrollData* sData = tCaller;

	gData.mCamera.mAddMovement(data, *sData);
}

void scrollBackgroundRight(double tAccel) {
	BackgroundScrollData sData = newBackgroundScrollData(makePosition(tAccel, 0, 0));

	list_map(&gData.mList, scrollSingleBackground, &sData);
}

void scrollBackgroundDown(double tAccel)
{
	BackgroundScrollData sData = newBackgroundScrollData(makePosition(0, tAccel, 0));
	list_map(&gData.mList, scrollSingleBackground, &sData);
}

Position* getScrollingBackgroundPositionReference(int tID) {
	SingleBackgroundData* data = list_get(&gData.mList, tID);

	return &data->mReferencedPosition;
}


void setScrollingBackgroundPosition(int tID, Position tPos) {
	SingleBackgroundData* data = list_get(&gData.mList, tID);
	data->mPhysics.mPosition = tPos;
	data->mReferencedPosition = tPos;
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
	data->mReferencedPosition = data->mPhysics.mPosition;
}

void addStageHandlerScreenShake(double tStrength)
{
	gData.mShake.mStrength += tStrength;
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


static void updatePhysicsCamera(SingleBackgroundData* tData) {
	setDragCoefficient(makePosition(0.1, 0.1, 0.1));
	setMaxVelocityDouble(tData->mMaxVelocity);
	handlePhysics(&tData->mPhysics);
	resetMaxVelocity();
	resetDragCoefficient();
}

static void addCameraPhysicsMovement(SingleBackgroundData* tData, BackgroundScrollData tScroll) {
	tData->mPhysics.mAcceleration = vecAdd(tData->mPhysics.mAcceleration, vecScale(tScroll.mAccel, tData->mScrollingFactor));
}

static void updateTweeningCamera(SingleBackgroundData* tData) {
	double tweeningFactor = 0.1;
	Vector3D delta = vecScale(vecSub(tData->mTweeningTarget, tData->mPhysics.mPosition), tweeningFactor);
	if (vecLength(delta) > tData->mMaxVelocity) {
		delta = vecScaleToSize(delta, tData->mMaxVelocity);	
	}

	tData->mPhysics.mPosition = vecAdd(tData->mPhysics.mPosition, delta);
}

static void addCameraTweeningMovement(SingleBackgroundData* tData, BackgroundScrollData tScroll) {
	tData->mTweeningTarget = vecAdd(tData->mTweeningTarget, vecScale(tScroll.mAccel, tData->mScrollingFactor));
	clampCameraRange(&tData->mTweeningTarget, tData->mScrollingFactor);
}

static StageHandlerCameraStrategy PhysicsCamera = {
	.mUpdate = updatePhysicsCamera,
	.mAddMovement = addCameraPhysicsMovement
};

static StageHandlerCameraStrategy TweeningCamera = {
	.mUpdate = updateTweeningCamera,
	.mAddMovement = addCameraTweeningMovement
};

void setStageHandlerAccelerationPhysics()
{
	gData.mCamera = PhysicsCamera;
}

static void setSingleBackgroundTweening(void* tCaller, void* tData) {
	(void)tCaller;
	SingleBackgroundData* data = tData;
	data->mTweeningTarget = data->mPhysics.mPosition;
}

void setStageHandlerTweening()
{
	gData.mCamera = TweeningCamera;
	list_map(&gData.mList, setSingleBackgroundTweening, NULL);
}

void setStageCameraRange(GeoRectangle tRange)
{
	gData.mCameraRange = tRange;
}