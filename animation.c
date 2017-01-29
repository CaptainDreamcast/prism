#include "include/animation.h"

#include "include/framerate.h"
#include "include/log.h"
#include "include/datastructures.h"

static struct {
	int mIsPaused;

} gData;

int getDurationInFrames(Duration tDuration){
	return tDuration * getInverseFramerateFactor();
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

	struct AnimationElement_internal* mPrev;
	struct AnimationElement_internal* mNext;

} AnimationElement;

typedef struct {
	int mSize;
	AnimationElement* mFirst;
	AnimationElement* mLast;
} AnimationList;

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
		if(cur->mIsLooped) {
			resetAnimation(&cur->mAnimation);
		} else {
			if(cur->mCB != NULL) {
				cur->mCB(cur->mCaller);
			}
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
	drawSprite(cur->mTextureData[frame], cur->mPosition, cur->mTexturePosition);	
}

void drawHandledAnimations() {
	list_map(&gAnimationHandler.mList, drawAnimationHandlerCB, NULL);
}
		
static void emptyAnimationHandler(){
	list_empty(&gAnimationHandler.mList);
}

static int playAnimationInternal(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller, int tIsLooped){
	
	AnimationElement* e = malloc(sizeof(AnimationElement));
	e->mCaller = tCaller;
	e->mCB = tOptionalCB;
	e->mIsLooped = tIsLooped;

	e->mPosition = tPosition;
	e->mTexturePosition = tTexturePosition;
	e->mTextureData = tTextures;
	e->mAnimation = tAnimation;

	return list_push_front_owned(&gAnimationHandler.mList, (void*)e);
}


int playAnimation(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller){
	return playAnimationInternal(tPosition, tTextures, tAnimation, tTexturePosition, tOptionalCB, tCaller, 0);	

}

int playAnimationLoop(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition){
	return playAnimationInternal(tPosition, tTextures, tAnimation, tTexturePosition, NULL, NULL, 1);
}

void removeHandledAnimation(int tID) {
	list_remove(&gAnimationHandler.mList, tID);
}

void shutdownAnimationHandler(){
	emptyAnimationHandler();
}
