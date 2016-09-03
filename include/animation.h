#ifndef TARI_ANIMATION
#define TARI_ANIMATION

#include <stdint.h>

#include "framerate.h"

typedef double Duration;
typedef uint32_t Frame;

typedef struct{
	Duration mNow;
	Duration mDuration;
	Frame mFrame;
	Frame mFrameAmount;
} Animation;

int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration);
void animate(Animation* tAnimation);
void resetAnimation(Animation* tAnimation);
void setAnimationFramerate(Framerate tFramerate);

#endif
