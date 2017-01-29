#ifndef TARI_ANIMATION
#define TARI_ANIMATION

#include <stdint.h>

#include "drawing.h"

typedef double Duration;
typedef uint32_t Frame;
typedef void (*AnimationPlayerCB)(void* caller);

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
Animation createOneFrameAnimation();
void pauseDurationHandling();
void resumeDurationHandling();

void setupAnimationHandler();
void updateAnimationHandler();
void drawHandledAnimations();
int playAnimation(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller);
int playAnimationLoop(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition);
void removeHandledAnimation(int tID);
void shutdownAnimationHandler();
#endif
