#pragma once

#include <stdint.h>

#include "drawing.h"

typedef double Duration;
typedef uint32_t Frame;
typedef void (*AnimationPlayerCB)(void* caller);
typedef uint32_t Tick;

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
int isDurationOver(Duration tNow, Duration tDuration);
int handleTickDurationAndCheckIfOver(Tick* tNow, Tick tDuration);
int isTickDurationOver(Tick tNow, Tick tDuration);

AnimationResult animateWithoutLoop(Animation* tAnimation);
void animate(Animation* tAnimation);
void resetAnimation(Animation* tAnimation);
Animation createAnimation(int tFrameAmount, Duration tDuration);
Animation createEmptyAnimation();
Animation createOneFrameAnimation();
void pauseDurationHandling();
void resumeDurationHandling();
double getDurationPercentage(Duration tNow, Duration tDuration);

void setupAnimationHandler();
void updateAnimationHandler();
void drawHandledAnimations();
int playAnimation(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller);
int playAnimationLoop(Position tPosition, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition);
int playOneFrameAnimationLoop(Position tPosition, TextureData* tTextures);

void changeAnimation(int tID, TextureData* tTextures, Animation tAnimation, Rectangle tTexturePosition);


void setAnimationScreenPositionReference(int tID, Position* tScreenPositionReference);
void setAnimationBasePositionReference(int tID, Position* tBasePositionReference);
void setAnimationScale(int tID, Vector3D tScale, Position tCenter);
void setAnimationSize(int tID, Vector3D tSize, Position tCenter);
void setAnimationRotationZ(int tID, double tAngle, Position tCenter);
void setAnimationColor(int tID, double r, double g, double b);
void setAnimationColorType(int tID, Color tColor);
void setAnimationTransparency(int tID, double a);
void setAnimationCB(int tID, AnimationPlayerCB tCB, void* tCaller);
void setAnimationPosition(int tID, Position tPosition);
void setAnimationTexturePosition(int tID, Rectangle tTexturePosition);
void setAnimationLoop(int tID, int tIsLooping);
void removeAnimationCB(int tID);
void fadeInAnimation(int tID, Duration tDuration);
void setAnimationCenter(int tID, Position tCenter);
void inverseAnimationVertical(int tID);
void inverseAnimationHorizontal(int tID);
void setAnimationVerticalInversion(int tID, int tValue);
void setAnimationHorizontalInversion(int tID, int tValue);
void setAnimationHandlerScreenRotationZ(double tAngle, Vector3D tCenter);
void setAnimationHandlerScreenTint(double r, double g, double b);
void resetAnimationHandlerScreenTint();

double* getAnimationTransparencyReference(int tID);
Position* getAnimationPositionReference(int tID);

void removeHandledAnimation(int tID);
int isHandledAnimation(int tID);
void shutdownAnimationHandler();
