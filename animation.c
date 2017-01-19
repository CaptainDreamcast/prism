#include "include/animation.h"

#include "include/framerate.h"

int getDurationInFrames(Duration tDuration){
	return tDuration * getInverseFramerateFactor();
}

int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration) {
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
