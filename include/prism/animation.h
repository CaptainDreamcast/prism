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

struct AnimationHandlerElement {
	int mID;
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

	int mIsScaled;
	Vector3D mScale;
	Position mScaleEffectCenter;

	int mIsRotated;
	double mRotationZ;
	Position mRotationEffectCenter;

	int mHasBaseColor;
	Vector3D mBaseColor;

	int mHasTransparency;
	double mTransparency;

	Vector3DI mInversionState;
	int mIsVisible;
};

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
AnimationHandlerElement* playAnimation(const Position& tPosition, TextureData* tTextures, const Animation& tAnimation, const Rectangle& tTexturePosition, AnimationPlayerCB tOptionalCB, void* tCaller);
AnimationHandlerElement* playAnimationLoop(const Position& tPosition, TextureData* tTextures, const Animation& tAnimation, const Rectangle& tTexturePosition);
AnimationHandlerElement* playOneFrameAnimationLoop(const Position& tPosition, TextureData* tTextures);

void changeAnimation(AnimationHandlerElement* tElement, TextureData* tTextures, const Animation& tAnimation, const Rectangle& tTexturePosition);


void setAnimationScreenPositionReference(AnimationHandlerElement* tElement, Position* tScreenPositionReference);
void setAnimationBasePositionReference(AnimationHandlerElement* tElement, Position* tBasePositionReference);
void setAnimationScale(AnimationHandlerElement* tElement, const Vector3D& tScale, const Position& tCenter);
void setAnimationSize(AnimationHandlerElement* tElement, const Vector3D& tSize, const Position& tCenter);
void setAnimationRotationZ(AnimationHandlerElement* tElement, double tAngle, const Position& tCenter);
void setAnimationColor(AnimationHandlerElement* tElement, double r, double g, double b);
void setAnimationColorType(AnimationHandlerElement* tElement, Color tColor);
void setAnimationTransparency(AnimationHandlerElement* tElement, double a);
void setAnimationVisibility(AnimationHandlerElement* tElement, int tIsVisible);
void setAnimationCB(AnimationHandlerElement* tElement, AnimationPlayerCB tCB, void* tCaller);
void setAnimationPosition(AnimationHandlerElement* tElement, const Position& tPosition);
void setAnimationTexturePosition(AnimationHandlerElement* tElement, const Rectangle& tTexturePosition);
void setAnimationLoop(AnimationHandlerElement* tElement, int tIsLooping);
void removeAnimationCB(AnimationHandlerElement* tElement);
void fadeInAnimation(AnimationHandlerElement* tElement, Duration tDuration);
void setAnimationCenter(AnimationHandlerElement* tElement, const Position& tCenter);
void inverseAnimationVertical(AnimationHandlerElement* tElement);
void inverseAnimationHorizontal(AnimationHandlerElement* tElement);
void setAnimationVerticalInversion(AnimationHandlerElement* tElement, int tValue);
void setAnimationHorizontalInversion(AnimationHandlerElement* tElement, int tValue);
void setAnimationHandlerScreenRotationZ(double tAngle, const Vector3D& tCenter);
void setAnimationHandlerScreenTint(double r, double g, double b);
void resetAnimationHandlerScreenTint();

double* getAnimationTransparencyReference(AnimationHandlerElement* tElement);
Position* getAnimationPositionReference(AnimationHandlerElement* tElement);

void removeHandledAnimation(AnimationHandlerElement* tElement);
int isHandledAnimation(AnimationHandlerElement* tElement);
void shutdownAnimationHandler();
