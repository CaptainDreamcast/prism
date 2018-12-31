#pragma once

#include <string>

#include "datastructures.h"
#include "animation.h"

#include "mugenspritefilereader.h"
#include "memorystack.h"

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

MugenAnimations loadMugenAnimationFile(std::string& tPath);
MugenAnimations loadMugenAnimationFile(char* tPath);
MugenAnimations loadMugenAnimationFileWithMemoryStack(char* tPath, MemoryStack* tMemoryStack);
void unloadMugenAnimationFile(MugenAnimations* tAnimations);

int hasMugenAnimation(MugenAnimations* tAnimations, int i);
MugenAnimation* getMugenAnimation(MugenAnimations* tAnimations, int i);
MugenAnimation* createOneFrameMugenAnimationForSprite(int tSpriteGroup, int tSpriteItem);
void destroyMugenAnimation(MugenAnimation* tAnimation);

Vector3DI  getAnimationFirstElementSpriteSize(MugenAnimation* tAnimation, MugenSpriteFile* tSprites);
Vector3D getAnimationFirstElementSpriteOffset(MugenAnimation * tAnimation, MugenSpriteFile* tSprites);
int isMugenAnimationStepDurationInfinite(int tDuration);