#include "include/animation.h"

#include <stdlib.h>

#include "include/framerate.h"
#include "include/log.h"
#include "include/datastructures.h"
#include "include/memoryhandler.h"
#include "include/system.h"

#include "include/timer.h" // TODO: separate animation and animation handler

static struct {
	int mIsPaused;

} gData;

int getDurationInFrames(Duration tDuration){
	return (int)(tDuration * getInverseFramerateFactor());
}

int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration) {
  if(gData.mIsPaused) return 0;
  (*tNow)++;
  if ((*tNow) >= getDurationInFrames(tDuration)) {
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
	Position mEffectCenter;
	int mIsScaled;
	Vector3D mScale;

	int mIsRotated;
	double mRotationZ;

	int mHasBaseColor;
	Vector3D mBaseColor;

	int mHasTransparency;
	double mTransparency;

	Vector3DI mInversionState;
	
} AnimationElement;

static struct{
	List mList;

} gAnimationHandler;

void setupAnimationHandler(){
	if(list_size(&gAnimationHandler.mList) > 0){
		logWarning("Setting up non-empty animation handler; Cleaning up.");
		shutdownAnimationHandler();
	}
	
	gAnimationHandler.mList = new_list();
}

static int updateAndRemoveCB(void* tCaller, void* tData) {
	(void) tCaller;
	AnimationElement* cur = tData;
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
	list_remove_predicate(&gAnimationHandler.mList, updateAndRemoveCB, NULL);
}

static void drawAnimationHandlerCB(void* tCaller, void* tData) {
	(void) tCaller;
	AnimationElement* cur = tData;
	int frame = cur->mAnimation.mFrame;
	Position p = cur->mPosition;
	if(cur->mScreenPositionReference != NULL) {
		p = vecAdd(p, vecScale(*cur->mScreenPositionReference, -1));
	}

	if(cur->mBasePositionReference != NULL) {
		p = vecAdd(p, *(cur->mBasePositionReference));
	}

	if(cur->mIsScaled) {
		Position sPosition = cur->mEffectCenter;
		sPosition = vecAdd(sPosition, p);
		scaleDrawing3D(cur->mScale, sPosition);
	}

	if (cur->mIsRotated) {
		Position rPosition = cur->mEffectCenter;
		rPosition = vecAdd(rPosition, p);
		setDrawingRotationZ(cur->mRotationZ, rPosition);
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

	drawSprite(cur->mTextureData[frame], p, texturePos);	

	if(cur->mIsScaled || cur->mIsRotated || cur->mHasBaseColor || cur->mHasTransparency) {
		setDrawingParametersToIdentity();
	}
}

void drawHandledAnimations() {
	list_map(&gAnimationHandler.mList, drawAnimationHandlerCB, NULL);
}
		
static void emptyAnimationHandler(){
	list_empty(&gAnimationHandler.mList);
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

	return list_push_front_owned(&gAnimationHandler.mList, (void*)e);
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
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);

	e->mTexturePosition = tTexturePosition;
	e->mTextureData = tTextures;
	e->mAnimation = tAnimation;
}

void setAnimationScreenPositionReference(int tID, Position* tScreenPositionReference) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mScreenPositionReference = tScreenPositionReference;

}

void setAnimationBasePositionReference(int tID, Position* tBasePositionReference) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mBasePositionReference = tBasePositionReference;
}

void setAnimationScale(int tID, Vector3D tScale, Position tCenter) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mIsScaled = 1;
	e->mEffectCenter = tCenter;
	e->mScale = tScale;
}

void setAnimationSize(int tID, Vector3D tSize, Position tCenter) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mIsScaled = 1;
	e->mEffectCenter = tCenter;

	double dx = tSize.x / e->mTextureData[0].mTextureSize.x;
	double dy = tSize.y / e->mTextureData[0].mTextureSize.y;
	e->mScale = makePosition(dx, dy, 1);
}

void setAnimationRotationZ(int tID, double tAngle, Position tCenter) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mIsRotated = 1;
	e->mEffectCenter = tCenter;
	e->mRotationZ = tAngle;
}

void setAnimationColor(int tID, double r, double g, double b) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mHasBaseColor = 1;
	e->mBaseColor = makePosition(r, g, b);
}

void setAnimationTransparency(int tID, double a) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mHasTransparency = 1;
	e->mTransparency = a;
}

void setAnimationCenter(int tID, Position tCenter) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mCenter = tCenter;
}

void setAnimationCB(int tID, AnimationPlayerCB tCB, void* tCaller) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mCB = tCB;
	e->mCaller = tCaller;
}

void setAnimationPosition(int tID, Position tPosition) {
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mPosition = tPosition;
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
	AnimationElement* e = list_get(&gAnimationHandler.mList, tID);
	e->mInversionState.x ^= 1;
}

void removeHandledAnimation(int tID) {
	list_remove(&gAnimationHandler.mList, tID);
}

void shutdownAnimationHandler(){
	emptyAnimationHandler();
}
