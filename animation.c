#include "include/animation.h"

#include <stdlib.h>

#include "include/framerate.h"
#include "include/log.h"
#include "include/datastructures.h"
#include "include/memoryhandler.h"

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

	if(cur->mIsScaled) {
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
