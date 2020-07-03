#include "prism/animation.h"

#include <stdlib.h>

#include "prism/log.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"

#include "prism/timer.h"
#include "prism/stlutil.h"

using namespace std;

static struct {
	int mIsPaused;

} gPrismAnimationData;

int getDurationInFrames(Duration tDuration){
	return (int)(tDuration * getInverseFramerateFactor());
}

int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration) {
  if(gPrismAnimationData.mIsPaused) return 0;
  (*tNow)++;
 
  return isDurationOver(*tNow, tDuration);
}

int isDurationOver(Duration tNow, Duration tDuration) {
	if (tNow >= getDurationInFrames(tDuration)) {
		return 1;
	}

	return 0;
}

int handleTickDurationAndCheckIfOver(Tick * tNow, Tick tDuration)
{
	if (gPrismAnimationData.mIsPaused) return 0;
	(*tNow)++;
	
	return isTickDurationOver(*tNow, tDuration);
}

int isTickDurationOver(Tick tNow, Tick tDuration)
{
	if (tNow >= tDuration) {
		return 1;
	}

	return 0;
}

AnimationResult animateWithoutLoop(Animation* tAnimation) {
	AnimationResult ret = ANIMATION_CONTINUING;
	if (handleDurationAndCheckIfOver(&tAnimation->mNow, tAnimation->mDuration)) {
		tAnimation->mNow = 0;
		tAnimation->mFrame++;
		if (tAnimation->mFrame >= tAnimation->mFrameAmount) {
			tAnimation->mFrame = tAnimation->mFrameAmount-1;
			tAnimation->mNow = getDurationInFrames(tAnimation->mDuration);
			ret = ANIMATION_OVER;
		}
	}

	return ret;
}

void animate(Animation* tAnimation) {
	AnimationResult ret = animateWithoutLoop(tAnimation);
	if(ret == ANIMATION_OVER){
		resetAnimation(tAnimation);
	}
}



void resetAnimation(Animation* tAnimation) {
  tAnimation->mNow = 0;
  tAnimation->mFrame = 0;
}

Animation createAnimation(int tFrameAmount, Duration tDuration) {
	Animation ret = createEmptyAnimation();
	ret.mFrameAmount = tFrameAmount;
	ret.mDuration = tDuration;
	return ret;
}

Animation createEmptyAnimation(){
  Animation ret;
  ret.mFrame = 0;
  ret.mFrameAmount = 0;
  ret.mNow = 0;
  ret.mDuration = 1000000000;
  return ret;
}


Animation createOneFrameAnimation(){
  Animation ret = createEmptyAnimation();
  ret.mFrameAmount = 1;
  return ret;
}


void pauseDurationHandling() {
	gPrismAnimationData.mIsPaused = 1;
}
void resumeDurationHandling() {
	gPrismAnimationData.mIsPaused = 0;
}

double getDurationPercentage(Duration tNow, Duration tDuration)
{
	int duration = getDurationInFrames(tDuration);
	return tNow / (double)duration;
}

static struct{
	map<int, AnimationHandlerElement> mList;
	int mIsLoaded;
} gAnimationHandler;

void setupAnimationHandler(){
	if(gAnimationHandler.mIsLoaded){
		logWarning("Setting up non-empty animation handler; Cleaning up.");
		shutdownAnimationHandler();
	}
	
	gAnimationHandler.mList.clear();
	gAnimationHandler.mIsLoaded = 1;
}

static int updateAndRemoveCB(void* tCaller, AnimationHandlerElement& tData) {
	(void) tCaller;
	AnimationHandlerElement* cur = &tData;
	AnimationResult res = animateWithoutLoop(&cur->mAnimation);
	if(res == ANIMATION_OVER) {
		if(cur->mCB != NULL) {
				cur->mCB(cur->mCaller);
		}
		if(cur->mIsLooped) {
			resetAnimation(&cur->mAnimation);
		} else {
			return 1;
		}
	}	
	return 0;
}

void updateAnimationHandler(){
	stl_int_map_remove_predicate(gAnimationHandler.mList, updateAndRemoveCB);
}

static Position getAnimationPositionWithAllReferencesIncluded(AnimationHandlerElement* cur) {
	Position p = cur->mPosition;
	if (cur->mScreenPositionReference != NULL) {
		p = vecAdd(p, vecScale(*cur->mScreenPositionReference, -1));
	}

	if (cur->mBasePositionReference != NULL) {
		p = vecAdd(p, *(cur->mBasePositionReference));
	}

	return p;
}

static void drawAnimationHandlerCB(void* tCaller, AnimationHandlerElement& tData) {
	(void) tCaller;
	AnimationHandlerElement* cur = &tData;
	if (!cur->mIsVisible) return;
	int frame = cur->mAnimation.mFrame;
	
	Position p = getAnimationPositionWithAllReferencesIncluded(cur);

	if (cur->mIsRotated) {
		Position rPosition = cur->mRotationEffectCenter;
		rPosition = vecAdd(rPosition, p);
		setDrawingRotationZ(cur->mRotationZ, rPosition);
	}

	if(cur->mIsScaled) {
		Position sPosition = cur->mScaleEffectCenter;
		sPosition = vecAdd(sPosition, p);
		scaleDrawing3D(cur->mScale, sPosition);
	}

	if (cur->mHasBaseColor) {
		setDrawingBaseColorAdvanced(cur->mBaseColor.x, cur->mBaseColor.y, cur->mBaseColor.z);
	}

	if (cur->mHasTransparency) {
		setDrawingTransparency(cur->mTransparency);
	}

	Rectangle texturePos = cur->mTexturePosition;

	if(cur->mInversionState.x) {
		Position center = vecAdd(cur->mCenter, p);
		double deltaX = center.x - p.x;
		double nRightX = center.x + deltaX;
		double nLeftX = nRightX - abs(cur->mTexturePosition.bottomRight.x - cur->mTexturePosition.topLeft.x);
		p.x = nLeftX;
		texturePos.topLeft.x = cur->mTexturePosition.bottomRight.x;
		texturePos.bottomRight.x = cur->mTexturePosition.topLeft.x;
	}

	if (cur->mInversionState.y) {
		Position center = vecAdd(cur->mCenter, p);
		double deltaY = center.y - p.y;
		double nDownY = center.y + deltaY;
		double nUpY = nDownY - abs(cur->mTexturePosition.bottomRight.y - cur->mTexturePosition.topLeft.y);
		p.y = nUpY;
		texturePos.topLeft.y = cur->mTexturePosition.bottomRight.y;
		texturePos.bottomRight.y = cur->mTexturePosition.topLeft.y;
	}

	drawSprite(cur->mTextureData[frame], p, texturePos);	

	if(cur->mIsScaled || cur->mIsRotated || cur->mHasBaseColor || cur->mHasTransparency) {
		setDrawingParametersToIdentity();
	}
}

void drawHandledAnimations() {
	stl_int_map_map(gAnimationHandler.mList, drawAnimationHandlerCB);
}
		
static void emptyAnimationHandler(){
	gAnimationHandler.mList.clear();
}

static AnimationHandlerElement* playAnimationInternal(const Position& tPosition, TextureData* tTextures, const Animation& tAnimation, const Rectangle& tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller, int tIsLooped){
	
	AnimationHandlerElement e;
	e.mCaller = tCaller;
	e.mCB = tOptionalCB;
	e.mIsLooped = tIsLooped;

	e.mPosition = tPosition;
	e.mTexturePosition = tTexturePosition;
	e.mTextureData = tTextures;
	e.mAnimation = tAnimation;
	e.mScreenPositionReference = NULL;
	e.mBasePositionReference = NULL;
	e.mIsScaled = 0;
	e.mIsRotated = 0;
	e.mHasBaseColor = 0;
	e.mHasTransparency = 0;
	e.mCenter = Vector3D(0,0,0);
	e.mInversionState = Vector3DI(0,0,0);
	e.mIsVisible = 1;
	int id = stl_int_map_push_back(gAnimationHandler.mList, e);
	auto& element = gAnimationHandler.mList[id];
	element.mID = id;
	return &element;
}


AnimationHandlerElement* playAnimation(const Position& tPosition, TextureData* tTextures, const Animation& tAnimation, const Rectangle& tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller){
	return playAnimationInternal(tPosition, tTextures, tAnimation, tTexturePosition, tOptionalCB, tCaller, 0);	

}

AnimationHandlerElement* playAnimationLoop(const Position& tPosition, TextureData* tTextures, const Animation& tAnimation, const Rectangle& tTexturePosition){
	return playAnimationInternal(tPosition, tTextures, tAnimation, tTexturePosition, NULL, NULL, 1);
}

AnimationHandlerElement* playOneFrameAnimationLoop(const Position& tPosition, TextureData* tTextures) {
	Animation anim = createOneFrameAnimation();
	Rectangle rect = makeRectangleFromTexture(tTextures[0]);
	return playAnimationLoop(tPosition, tTextures, anim, rect);
}

void changeAnimation(AnimationHandlerElement* e, TextureData* tTextures, const Animation& tAnimation, const Rectangle& tTexturePosition) {
	e->mTexturePosition = tTexturePosition;
	e->mTextureData = tTextures;
	e->mAnimation = tAnimation;
}

void setAnimationScreenPositionReference(AnimationHandlerElement* e, Position* tScreenPositionReference) {
	e->mScreenPositionReference = tScreenPositionReference;

}

void setAnimationBasePositionReference(AnimationHandlerElement* e, Position* tBasePositionReference) {
	e->mBasePositionReference = tBasePositionReference;
}

void setAnimationScale(AnimationHandlerElement* e, const Vector3D& tScale, const Position& tCenter) {

	e->mIsScaled = 1;
	e->mScaleEffectCenter = tCenter;
	e->mScale = tScale;
}

void setAnimationSize(AnimationHandlerElement* e, const Vector3D& tSize, const Position& tCenter) {

	e->mIsScaled = 1;
	e->mScaleEffectCenter = tCenter;

	double dx = tSize.x / e->mTextureData[0].mTextureSize.x;
	double dy = tSize.y / e->mTextureData[0].mTextureSize.y;
	e->mScale = Vector3D(dx, dy, 1);
}

static void setAnimationRotationZ_internal(AnimationHandlerElement* e, double tAngle, const Vector3D& tCenter) {
	e->mIsRotated = 1;
	e->mRotationEffectCenter = tCenter;
	e->mRotationZ = tAngle;
}

void setAnimationRotationZ(AnimationHandlerElement* e, double tAngle, const Position& tCenter) {

	setAnimationRotationZ_internal(e, tAngle, tCenter);
}

static void setAnimationColor_internal(AnimationHandlerElement* e, double r, double g, double b) {
	e->mHasBaseColor = 1;
	e->mBaseColor = Vector3D(r, g, b);
}

void setAnimationColor(AnimationHandlerElement* e, double r, double g, double b) {

	setAnimationColor_internal(e, r, g, b);
}

void setAnimationColorType(AnimationHandlerElement* e, Color tColor)
{
	double r, g, b;
	getRGBFromColor(tColor, &r, &g, &b);
	setAnimationColor(e, r, g, b);
}

void setAnimationTransparency(AnimationHandlerElement* e, double a) {
	e->mHasTransparency = 1;
	e->mTransparency = a;
}

void setAnimationVisibility(AnimationHandlerElement* e, int tIsVisible)
{
	e->mIsVisible = tIsVisible;
}

void setAnimationCenter(AnimationHandlerElement* e, const Position& tCenter) {
	e->mCenter = tCenter;
}

void setAnimationCB(AnimationHandlerElement* e, AnimationPlayerCB tCB, void* tCaller) {
	e->mCB = tCB;
	e->mCaller = tCaller;
}

void setAnimationPosition(AnimationHandlerElement* e, const Position& tPosition) {
	e->mPosition = tPosition;
}

void setAnimationTexturePosition(AnimationHandlerElement* e, const Rectangle& tTexturePosition)
{
	e->mTexturePosition = tTexturePosition;
}

void setAnimationLoop(AnimationHandlerElement* e, int tIsLooping) {

	e->mIsLooped = tIsLooping;
}

void removeAnimationCB(AnimationHandlerElement* e) {
	setAnimationCB(e, NULL, NULL);
}

typedef struct {
	AnimationHandlerElement* mElement;
	Vector3D mColor;
	Duration mDuration;
} AnimationColorIncrease;

static void increaseAnimationColor(void* tCaller) {
	AnimationColorIncrease* e = (AnimationColorIncrease*)tCaller;

	e->mColor = vecAdd(e->mColor, Vector3D(1.0 / e->mDuration, 1.0 / e->mDuration, 1.0 / e->mDuration));

	if (e->mColor.x >= 1) e->mColor = Vector3D(1, 1, 1);

	setAnimationColor(e->mElement, e->mColor.x, e->mColor.y, e->mColor.z);

	if (e->mColor.x >= 1) { freeMemory(e); }
	else addTimerCB(0,increaseAnimationColor, e);
}

void fadeInAnimation(AnimationHandlerElement* tElement, Duration tDuration) {
	AnimationColorIncrease* e = (AnimationColorIncrease*)allocMemory(sizeof(AnimationColorIncrease));
	e->mElement = tElement;
	e->mColor = Vector3D(0, 0, 0);
	e->mDuration = tDuration;
	addTimerCB(0, increaseAnimationColor, e);

	setAnimationColor(tElement, e->mColor.x, e->mColor.y, e->mColor.z);
}

void inverseAnimationVertical(AnimationHandlerElement* e) {
	e->mInversionState.x ^= 1;
}

void inverseAnimationHorizontal(AnimationHandlerElement* e) {
	e->mInversionState.y ^= 1;
}

void setAnimationVerticalInversion(AnimationHandlerElement* e, int tValue) {
	e->mInversionState.x = tValue;
}

void setAnimationHorizontalInversion(AnimationHandlerElement* e, int tValue) {
	e->mInversionState.y = tValue;
}

typedef struct {
	double mAngle;
	Vector3D mCenter;
} ScreenRotationZ;

static void setScreenRotationZForSingleAnimation(ScreenRotationZ* tRot, AnimationHandlerElement& tData) {
	AnimationHandlerElement* e = &tData;

	const auto p = getAnimationPositionWithAllReferencesIncluded(e);
	const auto center = vecSub(tRot->mCenter, p);
	setAnimationRotationZ_internal(e, tRot->mAngle, center);
}

void setAnimationHandlerScreenRotationZ(double tAngle, const Vector3D& tCenter)
{
	ScreenRotationZ rot;
	rot.mAngle = tAngle;
	rot.mCenter = tCenter;
	stl_int_map_map(gAnimationHandler.mList, setScreenRotationZForSingleAnimation, &rot);
}

typedef struct {
	double r;
	double g;
	double b;
} AnimationHandlerScreenTint;

static void setAnimationHandlerScreenTintSingle(AnimationHandlerScreenTint* tTint, AnimationHandlerElement& tData) {
	AnimationHandlerElement* e = &tData;
	setAnimationColor_internal(e, tTint->r, tTint->g, tTint->b);
}

void setAnimationHandlerScreenTint(double r, double g, double b)
{
	AnimationHandlerScreenTint tint;
	tint.r = r;
	tint.g = g;
	tint.b = b;

	stl_int_map_map(gAnimationHandler.mList, setAnimationHandlerScreenTintSingle, &tint);
}

void resetAnimationHandlerScreenTint()
{
	setAnimationHandlerScreenTint(1, 1, 1);
}

double* getAnimationTransparencyReference(AnimationHandlerElement* e)
{
	return &e->mTransparency;
}

Position* getAnimationPositionReference(AnimationHandlerElement* e) {
	return &e->mPosition;
}

void removeHandledAnimation(AnimationHandlerElement* e) {
	gAnimationHandler.mList.erase(e->mID);
}

int isHandledAnimation(AnimationHandlerElement* e) {
	return stl_map_contains(gAnimationHandler.mList, e->mID);
}

void shutdownAnimationHandler(){
	emptyAnimationHandler();
	gAnimationHandler.mList.clear();
	gAnimationHandler.mIsLoaded = 0;
}
