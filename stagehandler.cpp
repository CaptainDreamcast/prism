#include "prism/stagehandler.h"

#include <stdlib.h>
#include <algorithm>

#include "prism/script.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/file.h"
#include "prism/system.h"
#include "prism/log.h"
#include "prism/math.h"
#include "prism/texturepool.h"


using namespace std;

typedef struct {

	Acceleration mAccel;

} BackgroundScrollData;


typedef struct {

	Position mPosition;
	char mPath[100];

	Animation mAnimation;

	int mCanBeUnloaded;
	int mIsLoaded;
	TextureData* mTextureData;
	int mHasTextureSize;
	TextureSize mTextureSize;

	AnimationHandlerElement* mAnimationElement;
} BackgroundPatchData;

typedef struct {
	Vector3D mScrollingFactor;
	double mMaxVelocity;
	PhysicsObject mPhysics;
	Position mReferencedPosition;
	Position mTweeningTarget;
	double mZ;
	int mIsVisible;

	List mPatchList;
	ListIterator mCurrentStartPatch;
	ListIterator mCurrentEndPatch;
} SingleBackgroundData;

typedef void(*StageHandlerCameraUpdateFunction)(SingleBackgroundData* tData);
typedef void(*StageHandlerCameraAddMovementFunction)(SingleBackgroundData* tData, const BackgroundScrollData& tScroll);

typedef struct {
	StageHandlerCameraUpdateFunction mUpdate;
	StageHandlerCameraAddMovementFunction mAddMovement;
} StageHandlerCameraStrategy;

typedef struct {
	Position mDirection;
	double mStrength;
	double mMaximum;
} ScreenShake;

static struct {
	List mList;
	StageHandlerCameraStrategy mCamera;

	GeoRectangle2D mCameraRange;
	int mIsLoadingTexturesDirectly;
	ScreenShake mShake;
} gPrismStageHandlerData;

static void loadStageHandler(void*) {
	setProfilingSectionMarkerCurrentFunction();
	gPrismStageHandlerData.mList = new_list();
	gPrismStageHandlerData.mIsLoadingTexturesDirectly = 0;
	gPrismStageHandlerData.mCameraRange = GeoRectangle2D(-INF, -INF, INF * 2, INF * 2);
	gPrismStageHandlerData.mShake.mStrength = 0;
	gPrismStageHandlerData.mShake.mMaximum = INF;
	setStageHandlerAccelerationPhysics();
	
}

static void emptyAll(void* tCaller, void* tData) {
	(void)tCaller;
	SingleBackgroundData* data = (SingleBackgroundData*)tData;
	list_empty(&data->mPatchList);
}

static void unloadStageHandler(void*) {
	setProfilingSectionMarkerCurrentFunction();
	list_map(&gPrismStageHandlerData.mList, emptyAll, NULL);
	list_empty(&gPrismStageHandlerData.mList);
}

static void setPatchAnimationActive(BackgroundPatchData* tData, SingleBackgroundData* tBackgroundData) {
	tData->mAnimationElement = playAnimationLoop(tData->mPosition, tData->mTextureData, tData->mAnimation, makeRectangleFromTexture(tData->mTextureData[0]));
	setAnimationScreenPositionReference(tData->mAnimationElement, &tBackgroundData->mReferencedPosition);

}

static void setPatchAnimationInactive(BackgroundPatchData* tData) {
	removeHandledAnimation(tData->mAnimationElement);
}


static void loadStagePatchFromPath(BackgroundPatchData* tData) {
	tData->mTextureData = (TextureData*)allocMemory(tData->mAnimation.mFrameAmount * sizeof(TextureData));
	Frame i;
	TextureSize ts = { 0, 0 };
	for (i = 0; i < tData->mAnimation.mFrameAmount; i++) {
		char fPath[100];
		getPathWithNumberAffixedFromAssetPath(fPath, tData->mPath, i);
		tData->mTextureData[i] = loadTextureFromPool(fPath);
		ts.x = std::max(ts.x, tData->mTextureData[i].mTextureSize.x);
		ts.y = std::max(ts.y, tData->mTextureData[i].mTextureSize.y);
	}
	tData->mTextureSize = ts;
	tData->mHasTextureSize = 1;
}

static void loadStagePatchIfNecessary(BackgroundPatchData* tData, SingleBackgroundData* tBackgroundData) {
	if (tData->mIsLoaded) return;

	if (tData->mCanBeUnloaded) {
		loadStagePatchFromPath(tData);
	}

	
	if (tBackgroundData->mIsVisible) {
		setPatchAnimationActive(tData, tBackgroundData);
	}
	
	tData->mIsLoaded = 1;
}

static int isStagePatchOutOfBounds(BackgroundPatchData* tData, SingleBackgroundData* tBackgroundData) {
	if (gPrismStageHandlerData.mIsLoadingTexturesDirectly) return 0;

	double sl = tBackgroundData->mPhysics.mPosition.x;
	double sr = sl + 640.0;

	double l = tData->mPosition.x;
	int size;
	if (tData->mHasTextureSize) {
		size = tData->mTextureSize.x;
	}
	else {
		size = 640;
	}
	double r = l + size;

	int isOut = l > sr || r < sl;
	return isOut;
}



static void unloadStagePatchIfNecessary(BackgroundPatchData* tData, SingleBackgroundData* tBackgroundData) {
	if (!tData->mIsLoaded || !tData->mCanBeUnloaded) return;

	if (tBackgroundData->mIsVisible) {
		setPatchAnimationInactive(tData);
	}

	Frame i;
	for (i = 0; i < tData->mAnimation.mFrameAmount; i++) {
		unloadTextureFromPool(tData->mTextureData[i]);
	}
	freeMemory(tData->mTextureData);


	tData->mIsLoaded = 0;
}

static void setSingleStagePatches(SingleBackgroundData* data) {
	if (!data->mIsVisible) return;

	while (data->mCurrentEndPatch != NULL && !isStagePatchOutOfBounds((BackgroundPatchData*)list_iterator_get(data->mCurrentEndPatch), data)) {
		loadStagePatchIfNecessary((BackgroundPatchData*)list_iterator_get(data->mCurrentEndPatch), data);

		if (list_has_next(data->mCurrentEndPatch)) list_iterator_increase(&data->mCurrentEndPatch);
		else data->mCurrentEndPatch = NULL;

	}

	while (data->mCurrentStartPatch != NULL && isStagePatchOutOfBounds((BackgroundPatchData*)list_iterator_get(data->mCurrentStartPatch), data)) {
		unloadStagePatchIfNecessary((BackgroundPatchData*)list_iterator_get(data->mCurrentStartPatch), data);

		if (list_has_next(data->mCurrentStartPatch)) list_iterator_increase(&data->mCurrentStartPatch);
		else data->mCurrentStartPatch = NULL;
	}
}

static void clampCameraRange(Vector3D* v, const Vector3D& tScrollingFactor) {
	const auto rect = scaleGeoRectangleByFactor2D(gPrismStageHandlerData.mCameraRange, tScrollingFactor);
	*v = clampPositionToGeoRectangle(*v, rect);
}

static void setReferencedPosition(SingleBackgroundData* e) {
	double l = gPrismStageHandlerData.mShake.mStrength*vecLength(e->mScrollingFactor);
	e->mReferencedPosition = vecAdd(e->mPhysics.mPosition, vecScale(gPrismStageHandlerData.mShake.mDirection, l));
}

static void updateSingleStage(void* tCaller, void* tData) {
	(void)tCaller;
	SingleBackgroundData* data = (SingleBackgroundData*)tData;

	gPrismStageHandlerData.mCamera.mUpdate(data);
	clampCameraRange(&data->mPhysics.mPosition, data->mScrollingFactor);
	setReferencedPosition(data);

	setSingleStagePatches(data);
}

static void updateCameraShake() {
	gPrismStageHandlerData.mShake.mStrength *= 0.9;
	if (gPrismStageHandlerData.mShake.mStrength < 0.5) gPrismStageHandlerData.mShake.mStrength = 0;
	gPrismStageHandlerData.mShake.mStrength = min(gPrismStageHandlerData.mShake.mStrength, gPrismStageHandlerData.mShake.mMaximum);

	double angle = randfrom(0, M_PI * 2);
	gPrismStageHandlerData.mShake.mDirection = getDirectionFromAngleZ(angle);
}

static void updateStageHandler(void*) {
	setProfilingSectionMarkerCurrentFunction();
	updateCameraShake();
	list_map(&gPrismStageHandlerData.mList, updateSingleStage, NULL);
}

ActorBlueprint getStageHandler()
{
	return makeActorBlueprint(loadStageHandler, unloadStageHandler, updateStageHandler);
}

void setStageHandlerNoDelayedLoading() {
	gPrismStageHandlerData.mIsLoadingTexturesDirectly = 1;
}

int addScrollingBackground(double tScrollingFactor, double tZ) {
	return addScrollingBackgroundWithMovementIn2D(tScrollingFactor, tScrollingFactor, tZ);
}

int addScrollingBackgroundWithMovementIn2D(double tDeltaX, double tDeltaY, double tZ)
{
	SingleBackgroundData* data = (SingleBackgroundData*)allocMemory(sizeof(SingleBackgroundData));
	data->mScrollingFactor = Vector3D(tDeltaX, tDeltaY, 0);
	data->mMaxVelocity = INF;
	resetPhysicsObject(&data->mPhysics);
	data->mPhysics.mPosition = Vector3D(0, 0, 0);
	data->mTweeningTarget = data->mPhysics.mPosition;
	data->mReferencedPosition = data->mPhysics.mPosition;
	data->mZ = tZ;
	data->mIsVisible = 1;
	data->mPatchList = new_list();
	data->mCurrentStartPatch = NULL;
	data->mCurrentEndPatch = NULL;


	return list_push_front_owned(&gPrismStageHandlerData.mList, data);
}

int addBackgroundElementInternal(int tBackgroundID, const Position& tPosition, const char* tPath, TextureData* tTextureData, const Animation& tAnimation, int tCanBeUnloaded) {
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tBackgroundID);

	BackgroundPatchData* pData = (BackgroundPatchData*)allocMemory(sizeof(BackgroundPatchData));
	
	pData->mCanBeUnloaded = tCanBeUnloaded;
	pData->mIsLoaded = 0;
	pData->mHasTextureSize = 0;
	pData->mTextureData = tTextureData;
	pData->mPosition = tPosition;
	pData->mPosition.z = data->mZ + list_size(&data->mPatchList)*0.001;
	pData->mAnimation = tAnimation;
	strcpy(pData->mPath, tPath);

	int id = list_push_back_owned(&data->mPatchList, pData);

	if (data->mCurrentStartPatch == NULL) data->mCurrentStartPatch = data->mCurrentEndPatch = list_iterator_begin(&data->mPatchList);

	if (gPrismStageHandlerData.mIsLoadingTexturesDirectly || !pData->mCanBeUnloaded) {
		loadStagePatchIfNecessary(pData, data);
	}

	return id;

}

int addBackgroundElement(int tBackgroundID, const Position& tPosition, char* tPath, const Animation& tAnimation) {
	return addBackgroundElementInternal(tBackgroundID, tPosition, tPath, NULL, tAnimation, 1);
}

int addBackgroundElementWithTextureData(int tBackgroundID, const Position& tPosition, TextureData * tTextureData, const Animation& tAnimation)
{
	return addBackgroundElementInternal(tBackgroundID, tPosition, "", tTextureData, tAnimation, 0);
}

TextureData* getBackgroundElementTextureData(int tBackgroundID, int tElementID)
{
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tBackgroundID);
	BackgroundPatchData* e = (BackgroundPatchData*)list_get(&data->mPatchList, tElementID);
	return e->mTextureData;
}

Position getRealScreenPosition(int tBackgroundID, const Position& tPos) {
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tBackgroundID);
	Position p = vecAdd(tPos, vecScale(data->mReferencedPosition, -1));
	p.z = tPos.z;
	return p;
}

static BackgroundScrollData newBackgroundScrollData(const Acceleration& tAccel) {
	BackgroundScrollData ret;
	ret.mAccel = tAccel;
	return ret;
}

static void scrollSingleBackground(void* tCaller, void* tData) {
	SingleBackgroundData* data = (SingleBackgroundData*)tData;
	BackgroundScrollData* sData = (BackgroundScrollData*)tCaller;

	gPrismStageHandlerData.mCamera.mAddMovement(data, *sData);
}

void scrollBackgroundRight(double tAccel) {
	BackgroundScrollData sData = newBackgroundScrollData(Vector3D(tAccel, 0, 0));

	list_map(&gPrismStageHandlerData.mList, scrollSingleBackground, &sData);
}

void scrollBackgroundDown(double tAccel)
{
	BackgroundScrollData sData = newBackgroundScrollData(Vector3D(0, tAccel, 0));
	list_map(&gPrismStageHandlerData.mList, scrollSingleBackground, &sData);
}

Position* getScrollingBackgroundPositionReference(int tID) {
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tID);

	return &data->mReferencedPosition;
}


static void resetScrollingBackgroundPatchLoading(SingleBackgroundData* data) {
	data->mCurrentStartPatch = data->mCurrentEndPatch = list_iterator_begin(&data->mPatchList);
	setSingleStagePatches(data);
}

void setScrollingBackgroundPosition(int tID, const Position& tPos) {
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tID);
	data->mPhysics.mPosition = tPos;
	data->mReferencedPosition = tPos;
	resetScrollingBackgroundPatchLoading(data);
}

void setScrollingBackgroundMaxVelocity(int tID, double tVel) {
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tID);
	data->mMaxVelocity = tVel;
}

PhysicsObject* getScrollingBackgroundPhysics(int tID) {
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tID);
	return &data->mPhysics;
}

void setScrollingBackgroundPhysics(int tID, const PhysicsObject& tPhysics) {
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tID);
	data->mPhysics = tPhysics;
	data->mReferencedPosition = data->mPhysics.mPosition;
}

static void setStagePatchVisible(void* tCaller, void* tData) {
	SingleBackgroundData* data = (SingleBackgroundData*)tCaller;
	BackgroundPatchData* e = (BackgroundPatchData*)tData;

	if (!e->mIsLoaded) return;

	setPatchAnimationActive(e, data);
}

static void setStagePatchInvisible(void* tCaller, void* tData) {
	(void) tCaller;
	BackgroundPatchData* e = (BackgroundPatchData*)tData;

	if (!e->mIsLoaded) return;

	setPatchAnimationInactive(e);
}

void setScrollingBackgroundInvisible(int tID)
{
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tID);
	data->mIsVisible = 0;
	list_map(&data->mPatchList, setStagePatchInvisible, data);
}

void setScrollingBackgroundVisible(int tID)
{
	SingleBackgroundData* data = (SingleBackgroundData*)list_get(&gPrismStageHandlerData.mList, tID);
	data->mIsVisible = 1;
	list_map(&data->mPatchList, setStagePatchVisible, data);
}

void addStageHandlerScreenShake(double tStrength)
{
	gPrismStageHandlerData.mShake.mStrength += tStrength;
}

void setStageHandlerMaximumScreenShake(double tStrength)
{
	gPrismStageHandlerData.mShake.mMaximum = tStrength;
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

static ScriptPosition loadStageScriptLayerElement(void* tCaller, const ScriptPosition& tPos) {
	StageScriptLayerElementData* e = (StageScriptLayerElementData*)tCaller;
	char word[1024];

	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("POSITION", word)) {
		ret = getNextScriptDouble(ret, &e->mPosition.x);
		ret = getNextScriptDouble(ret, &e->mPosition.y);
		e->mPosition.z = 0;
	}
	else if (!strcmp("PATH", word)) {
		ret = getNextScriptString(ret, e->mPath);
	}
	else if (!strcmp("ANIMATION", word)) {
		e->mAnimation = createEmptyAnimation();

		int v;
		ret = getNextScriptInteger(ret, &v);
		e->mAnimation.mDuration = v;

		ret = getNextScriptInteger(ret, &v);
		e->mAnimation.mFrameAmount = v;
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
}

static ScriptPosition loadStageScriptLayer(void* tCaller, const ScriptPosition& tPos) {
	StageScriptLayerData* e = (StageScriptLayerData*)tCaller;
	char word[1024];

	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("POSITION_Z", word)) {
		ret = getNextScriptDouble(ret, &e->mZ);
	}
	else if (!strcmp("SCROLLING_FACTOR", word)) {
		ret = getNextScriptDouble(ret, &e->mScrollingFactor);
	}
	else if (!strcmp("MAX_VELOCITY", word)) {
		ret = getNextScriptDouble(ret, &e->mMaxVelocity);
	}
	else if (!strcmp("CREATE", word)) {
		e->mID = addScrollingBackground(e->mScrollingFactor, e->mZ);
		setScrollingBackgroundMaxVelocity(e->mID, e->mMaxVelocity);
	}
	else if (!strcmp("ELEMENT", word)) {
		ScriptRegion reg = getScriptRegionAtPosition(ret);
		StageScriptLayerElementData caller;
		memset(&caller, 0, sizeof(StageScriptLayerElementData));
		executeOnScriptRegion(reg, loadStageScriptLayerElement, &caller);
		ret = getPositionAfterScriptRegion(ret.mRegion, reg);
		addBackgroundElement(e->mID, caller.mPosition, caller.mPath, caller.mAnimation);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
}

static ScriptPosition loadStageScriptStage(void* tCaller, const ScriptPosition& tPos) {
	(void)tCaller;
	char word[1024];

	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("LAYER", word)) {
		ScriptRegion reg = getScriptRegionAtPosition(ret);
		StageScriptLayerData caller;
		memset(&caller, 0, sizeof(StageScriptLayerData));
		caller.mMaxVelocity = INF;
		executeOnScriptRegion(reg, loadStageScriptLayer, &caller);
		ret = getPositionAfterScriptRegion(ret.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
}

void loadStageFromScript(const char* tPath) {
	Script s = loadScript(tPath);
	ScriptRegion reg = getScriptRegion(s, "STAGE");

	executeOnScriptRegion(reg, loadStageScriptStage, NULL);
}

static void updatePhysicsCamera(SingleBackgroundData* tData) {
	setDragCoefficient(Vector3D(0.1, 0.1, 0.1));
	setMaxVelocityDouble(tData->mMaxVelocity);
	handlePhysics(&tData->mPhysics);
	resetMaxVelocity();
	resetDragCoefficient();
}

static void addCameraPhysicsMovement(SingleBackgroundData* tData, const BackgroundScrollData& tScroll) {
	tData->mPhysics.mAcceleration = vecAdd(tData->mPhysics.mAcceleration, vecScale3D(tScroll.mAccel, tData->mScrollingFactor));
}

static void updateTweeningCamera(SingleBackgroundData* tData) {
	double tweeningFactor = 0.1;
	Vector3D delta = vecScale(vecSub(tData->mTweeningTarget, tData->mPhysics.mPosition), tweeningFactor);
	if (vecLength(delta) > tData->mMaxVelocity) {
		delta = vecScaleToSize(delta, tData->mMaxVelocity);	
	}

	tData->mPhysics.mPosition = vecAdd(tData->mPhysics.mPosition, delta);
}

static void addCameraTweeningMovement(SingleBackgroundData* tData, const BackgroundScrollData& tScroll) {
	tData->mTweeningTarget = vecAdd(tData->mTweeningTarget, vecScale3D(tScroll.mAccel, tData->mScrollingFactor));
	clampCameraRange(&tData->mTweeningTarget, tData->mScrollingFactor);
}

static StageHandlerCameraStrategy getPhysicsCamera() {
	StageHandlerCameraStrategy ret;
	ret.mUpdate = updatePhysicsCamera;
	ret.mAddMovement = addCameraPhysicsMovement;
	return ret;
}

static StageHandlerCameraStrategy getTweeningCamera() {
	StageHandlerCameraStrategy ret;
	ret.mUpdate = updateTweeningCamera;
	ret.mAddMovement = addCameraTweeningMovement;
	return ret;
}

void setStageHandlerAccelerationPhysics()
{
	gPrismStageHandlerData.mCamera = getPhysicsCamera();
}

static void setSingleBackgroundTweening(void* tCaller, void* tData) {
	(void)tCaller;
	SingleBackgroundData* data = (SingleBackgroundData*)tData;
	data->mTweeningTarget = data->mPhysics.mPosition;
}

void setStageHandlerTweening()
{
	gPrismStageHandlerData.mCamera = getTweeningCamera();
	list_map(&gPrismStageHandlerData.mList, setSingleBackgroundTweening, NULL);
}

void setStageCameraRange(const GeoRectangle2D& tRange)
{
	gPrismStageHandlerData.mCameraRange = tRange;
}
