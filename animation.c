#include "include/animation.h"

Framerate gFramerate;

int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration){
	(*tNow)++;
	if((*tNow) >= tDuration){
		return 1;
	}

	return 0;
}

void animate(Animation* tAnimation){
	if(handleDurationAndCheckIfOver(&tAnimation->mNow, tAnimation->mDuration)){
		tAnimation->mNow = 0;
		tAnimation->mFrame++;
		if(tAnimation->mFrame >= tAnimation->mFrameAmount){
			tAnimation->mFrame = 0;
		}
	}
}

void resetAnimation(Animation* tAnimation){
	tAnimation->mNow = 0;
	tAnimation->mFrame = 0;
}

void setFramerate(Framerate tFramerate){
	gFramerate = tFramerate;
}
