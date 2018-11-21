#include "prism/animation.h"

#include <stdlib.h>

#include "prism/framerate.h"
#include "prism/log.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/system.h"

#include "prism/timer.h" // TODO: separate animation and animation handler

static struct {
	int mIsPaused;

} gData;

int getDurationInFrames(Duration tDuration){
	return (int)(tDuration * getInverseFramerateFactor());
}

int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration) {
  if(gData.mIsPaused) return 0;
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
	if (gData.mIsPaused) return 0;
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
	gData.mIsPaused = 1;
}
void resumeDurationHandling() {
	gData.mIsPaused = 0;
}

double getDurationPercentage(Duration tNow, Duration tDuration)
{
	int duration = getDurationInFrames(tDuration);
	return tNow / (double)duration;
}

typedef struct AnimationElement_internal {
	
	void* mCaller;
	AnimationPlayerCB mCB;

	Position mPosition;
	Rectangle mTexturePosition;
	TextureData* mTextureData;
	Animation mAnimation;
	int mIsLooped;	

	Position* mScreenPositionReference;
	Position* mBasePositionReference;

	Position mCenter;
	
	int mIsScaled;
	Vector3D mScale;
	Position mScaleEffectCenter;

	int mIsRotated;
	double mRotationZ;
	Position mRotationEffectCenter;

	int mHasBaseColor;
	Vector3D mBaseColor;

	int mHasTransparency;
	double mTransparency;

	Vector3DI mInversionState;
	
} AnimationElement;

static struct{
	IntMap mList;
	int mIsLoaded;
} gAnimationHandler;

void setupAnimationHandler(){
	if(gAnimationHandler.mIsLoaded){
		logWarning("Setting up non-empty animation handler; Cleaning up.");
		shutdownAnimationHandler();
	}
	
	gAnimationHandler.mList = new_int_map();
	gAnimationHandler.mIsLoaded = 1;
}

static int updateAndRemoveCB(void* tCaller, void* tData) {
	(void) tCaller;
	AnimationElement* cur = (AnimationElement*)tData;
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
	int_map_remove_predicate(&gAnimationHandler.mList, updateAndRemoveCB, NULL);
}

static Position getAnimationPositionWithAllReferencesIncluded(AnimationElement* cur) {
	Position p = cur->mPosition;
	if (cur->mScreenPositionReference != NULL) {
		p = vecAdd(p, vecScale(*cur->mScreenPositionReference, -1));
	}

	if (cur->mBasePositionReference != NULL) {
		p = vecAdd(p, *(cur->mBasePositionReference));
	}

	return p;
}

static void drawAnimationHandlerCB(void* tCaller, void* tData) {
	(void) tCaller;
	AnimationElement* cur = tData;
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
	int_map_map(&gAnimationHandler.mList, drawAnimationHandlerCB, NULL);
}
		
static void emptyAnimationHandler(){
	int_map_empty(&gAnimationHandler.mList);
}

static int playAnimationInternal(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller, int tIsLooped){
	
	AnimationElement* e = allocMemory(sizeof(AnimationElement));
	e->mCaller = tCaller;
	e->mCB = tOptionalCB;
	e->mIsLooped = tIsLooped;

	e->mPosition = tPosition;
	e->mTexturePosition = tTexturePosition;
	e->mTextureData = tTextures;
	e->mAnimation = tAnimation;
	e->mScreenPositionReference = NULL;
	e->mBasePositionReference = NULL;
	e->mIsScaled = 0;
	e->mIsRotated = 0;
	e->mHasBaseColor = 0;
	e->mHasTransparency = 0;
	e->mCenter = makePosition(0,0,0);
	e->mInversionState = makeVector3DI(0,0,0);

	return int_map_push_back_owned(&gAnimationHandler.mList, (void*)e);
}


int playAnimation(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller){
	return playAnimationInternal(tPosition, tTextures, tAnimation, tTexturePosition, tOptionalCB, tCaller, 0);	

}

int playAnimationLoop(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition){
	return playAnimationInternal(tPosition, tTextures, tAnimation, tTexturePosition, NULL, NULL, 1);
}

int playOneFrameAnimationLoop(Position tPosition, TextureData* tTextures) {
	Animation anim = createOneFrameAnimation();
	Rectangle rect = makeRectangleFromTexture(tTextures[0]);
	return playAnimationLoop(tPosition, tTextures, anim, rect);
}

void changeAnimation(int tID, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);

	e->mTexturePosition = tTexturePosition;
	e->mTextureData = tTextures;
	e->mAnimation = tAnimation;
}

void setAnimationScreenPositionReference(int tID, Position* tScreenPositionReference) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mScreenPositionReference = tScreenPositionReference;

}

void setAnimationBasePositionReference(int tID, Position* tBasePositionReference) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mBasePositionReference = tBasePositionReference;
}

void setAnimationScale(int tID, Vector3D tScale, Position tCenter) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mIsScaled = 1;
	e->mScaleEffectCenter = tCenter;
	e->mScale = tScale;
}

void setAnimationSize(int tID, Vector3D tSize, Position tCenter) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mIsScaled = 1;
	e->mScaleEffectCenter = tCenter;

	double dx = tSize.x / e->mTextureData[0].mTextureSize.x;
	double dy = tSize.y / e->mTextureData[0].mTextureSize.y;
	e->mScale = makePosition(dx, dy, 1);
}

static void setAnimationRotationZ_internal(AnimationElement* e, double tAngle, Vector3D tCenter) {
	e->mIsRotated = 1;
	e->mRotationEffectCenter = tCenter;
	e->mRotationZ = tAngle;
}

void setAnimationRotationZ(int tID, double tAngle, Position tCenter) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	setAnimationRotationZ_internal(e, tAngle, tCenter);
}

static void setAnimationColor_internal(AnimationElement* e, double r, double g, double b) {
	e->mHasBaseColor = 1;
	e->mBaseColor = makePosition(r, g, b);
}

void setAnimationColor(int tID, double r, double g, double b) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	setAnimationColor_internal(e, r, g, b);
}

void setAnimationColorType(int tID, Color tColor)
{
	double r, g, b;
	getRGBFromColor(tColor, &r, &g, &b);
	setAnimationColor(tID, r, g, b);
}

void setAnimationTransparency(int tID, double a) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mHasTransparency = 1;
	e->mTransparency = a;
}

void setAnimationCenter(int tID, Position tCenter) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mCenter = tCenter;
}

void setAnimationCB(int tID, AnimationPlayerCB tCB, void* tCaller) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mCB = tCB;
	e->mCaller = tCaller;
}

void setAnimationPosition(int tID, Position tPosition) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mPosition = tPosition;
}

void setAnimationTexturePosition(int tID, Rectangle tTexturePosition)
{
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mTexturePosition = tTexturePosition;
}

void setAnimationLoop(int tID, int tIsLooping) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mIsLooped = tIsLooping;
}

void removeAnimationCB(int tID) {
	setAnimationCB(tID, NULL, NULL);
}

typedef struct {
	int mID;
	Vector3D mColor;
	Duration mDuration;
} AnimationColorIncrease;

static void increaseAnimationColor(void* tCaller) {
	AnimationColorIncrease* e = tCaller;

	e->mColor = vecAdd(e->mColor, makePosition(1.0 / e->mDuration, 1.0 / e->mDuration, 1.0 / e->mDuration));

	if (e->mColor.x >= 1) e->mColor = makePosition(1, 1, 1);

	setAnimationColor(e->mID, e->mColor.x, e->mColor.y, e->mColor.z);

	if (e->mColor.x >= 1) { freeMemory(e); }
	else addTimerCB(0,increaseAnimationColor, e);
}

void fadeInAnimation(int tID, Duration tDuration) {
	AnimationColorIncrease* e = allocMemory(sizeof(AnimationColorIncrease));
	e->mID = tID;
	e->mColor = makePosition(0, 0, 0);
	e->mDuration = tDuration;
	addTimerCB(0, increaseAnimationColor, e);

	setAnimationColor(e->mID, e->mColor.x, e->mColor.y, e->mColor.z);
}

void inverseAnimationVertical(int tID) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mInversionState.x ^= 1;
}

void inverseAnimationHorizontal(int tID) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mInversionState.y ^= 1;
}

void setAnimationVerticalInversion(int tID, int tValue) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mInversionState.x = tValue;
}

void setAnimationHorizontalInversion(int tID, int tValue) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	e->mInversionState.y = tValue;
}

typedef struct {
	double mAngle;
	Vector3D mCenter;
} ScreenRotationZ;

static void setScreenRotationZForSingleAnimation(void* tCaller, void* tData) {
	ScreenRotationZ* rot = tCaller;
	AnimationElement* e = tData;

	Position p = getAnimationPositionWithAllReferencesIncluded(e);
	Position center = vecSub(rot->mCenter, p);
	setAnimationRotationZ_internal(e, rot->mAngle, center);
}

void setAnimationHandlerScreenRotationZ(double tAngle, Vector3D tCenter)
{
	ScreenRotationZ rot;
	rot.mAngle = tAngle;
	rot.mCenter = tCenter;
	int_map_map(&gAnimationHandler.mList, setScreenRotationZForSingleAnimation, &rot);
}

typedef struct {
	double r;
	double g;
	double b;
} AnimationHandlerScreenTint;

static void setAnimationHandlerScreenTintSingle(void* tCaller, void* tData) {
	AnimationHandlerScreenTint* tint = tCaller;
	AnimationElement* e = tData;
	setAnimationColor_internal(e, tint->r, tint->g, tint->b);
}

void setAnimationHandlerScreenTint(double r, double g, double b)
{
	AnimationHandlerScreenTint tint;
	tint.r = r;
	tint.g = g;
	tint.b = b;

	int_map_map(&gAnimationHandler.mList, setAnimationHandlerScreenTintSingle, &tint);
}

void resetAnimationHandlerScreenTint()
{
	setAnimationHandlerScreenTint(1, 1, 1);
}

double* getAnimationTransparencyReference(int tID)
{
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	return &e->mTransparency;
}

Position* getAnimationPositionReference(int tID) {
	AnimationElement* e = int_map_get(&gAnimationHandler.mList, tID);
	return &e->mPosition;
}

void removeHandledAnimation(int tID) {
	int_map_remove(&gAnimationHandler.mList, tID);
}

int isHandledAnimation(int tID) {
	return int_map_contains(&gAnimationHandler.mList, tID);
}

void shutdownAnimationHandler(){
	emptyAnimationHandler();
	delete_int_map(&gAnimationHandler.mList);
	gAnimationHandler.mIsLoaded = 0;
}
