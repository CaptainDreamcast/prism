#pragma once

#include "tari/datastructures.h"
#include "tari/animation.h"

#include "tari/mugenspritefilereader.h"

typedef enum {
	FACE_DIRECTION_LEFT,
	FACE_DIRECTION_RIGHT
} FaceDirection;


typedef struct {
	List mPassiveHitboxes; // contain CollisionRect
	List mAttackHitboxes;
	int mGroupNumber;
	int mSpriteNumber;
	Vector3D mDelta;
	int mDuration;
	int mIsFlippingHorizontally;
	int mIsFlippingVertically;

	int mIsAddition;
	int mIsSubtraction;
	int mSrcBlendValue;
	int mDstBlendValue;

	int mInterpolateOffset;
	int mInterpolateBlend;
	int mInterpolateScale;
	int mInterpolateAngle;
} MugenAnimationStep;

typedef struct {
	int mID;
	int mLoopStart;
	int mTotalDuration;
	Vector mSteps;
} MugenAnimation;

typedef struct {
	IntMap mAnimations;

} MugenAnimations;

fup MugenAnimations loadMugenAnimationFile(char* tPath);

fup int hasMugenAnimation(MugenAnimations* tAnimations, int i);
fup MugenAnimation* getMugenAnimation(MugenAnimations* tAnimations, int i);
fup MugenAnimation* createOneFrameMugenAnimationForSprite(int tSpriteGroup, int tSpriteItem);
fup void destroyMugenAnimation(MugenAnimation* tAnimation);

fup Vector3DI  getAnimationFirstElementSpriteSize(MugenAnimation* tAnimation, MugenSpriteFile* tSprites);