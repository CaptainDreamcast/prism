#ifndef TARI_ANIMATION
#define TARI_ANIMATION

#include <stdint.h>

typedef double Duration;
typedef uint32_t Frame;

typedef enum {
	ANIMATION_OVER,
	ANIMATION_CONTINUING
} AnimationResult;

typedef struct {
  Duration mNow;
  Duration mDuration;
  Frame mFrame;
  Frame mFrameAmount;
} Animation;

int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration);
AnimationResult animateWithoutLoop(Animation* tAnimation);
void animate(Animation* tAnimation);
void resetAnimation(Animation* tAnimation);
Animation createEmptyAnimation();

#endif
