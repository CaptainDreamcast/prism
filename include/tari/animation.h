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

fup int handleDurationAndCheckIfOver(Duration* tNow, Duration tDuration);
fup AnimationResult animateWithoutLoop(Animation* tAnimation);
fup void animate(Animation* tAnimation);
fup void resetAnimation(Animation* tAnimation);
fup Animation createEmptyAnimation();
fup Animation createOneFrameAnimation();
fup void pauseDurationHandling();
fup void resumeDurationHandling();
fup double getDurationPercentage(Duration tNow, Duration tDuration);

fup void setupAnimationHandler();
fup void updateAnimationHandler();
fup void drawHandledAnimations();
fup int playAnimation(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller);
fup int playAnimationLoop(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition);
fup int playOneFrameAnimationLoop(Position tPosition, TextureData* tTextures);

fup void changeAnimation(int tID, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition);


fup void setAnimationScreenPositionReference(int tID, Position* tScreenPositionReference);
fup void setAnimationBasePositionReference(int tID, Position* tBasePositionReference);
fup void setAnimationScale(int tID, Vector3D tScale, Position tCenter);
fup void setAnimationSize(int tID, Vector3D tSize, Position tCenter);
fup void setAnimationRotationZ(int tID, double tAngle, Position tCenter);
fup void setAnimationColor(int tID, double r, double g, double b);
fup void setAnimationColorType(int tID, Color tColor);
fup void setAnimationTransparency(int tID, double a);
fup void setAnimationCB(int tID, AnimationPlayerCB tCB, void* tCaller);
fup void setAnimationPosition(int tID, Position tPosition);
fup void removeAnimationCB(int tID);
fup void fadeInAnimation(int tID, Duration tDuration);
fup void setAnimationCenter(int tID, Position tCenter);
fup void inverseAnimationVertical(int tID);

fup void removeHandledAnimation(int tID);
fup void shutdownAnimationHandler();
#endif
