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

enum class MugenAnimationStepFlags : uint32_t {
	NONE =						0,
	IS_FLIPPING_HORIZONTALLY =	(1 << 0),
	IS_FLIPPING_VERTICALLY =	(1 << 1),
	IS_ADDITION =				(1 << 2),
	IS_SUBTRACTION =			(1 << 3),
};

typedef struct {
	List mPassiveHitboxes; // contain CollisionRect
	List mAttackHitboxes;
	int mGroupNumber;
	int mSpriteNumber;
	Vector2D mDelta;
	int mDuration;

	uint32_t mFlags;
	double mSrcBlendFactor;
	double mDstBlendFactor;

	double mScaleX;
	double mScaleY;
	double mAngleRad;

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

MugenAnimations loadMugenAnimationFile(const std::string& tPath);
MugenAnimations loadMugenAnimationFile(const char* tPath);
void unloadMugenAnimationFile(MugenAnimations* tAnimations);

int hasMugenAnimation(MugenAnimations* tAnimations, int i);
MugenAnimation* getMugenAnimation(MugenAnimations* tAnimations, int i);
MugenAnimation* createOneFrameMugenAnimationForSprite(int tSpriteGroup, int tSpriteItem);
void destroyMugenAnimation(MugenAnimation* tAnimation);

Vector2DI getAnimationFirstElementSpriteSize(MugenAnimation* tAnimation, MugenSpriteFile* tSprites);
Vector2D getAnimationFirstElementSpriteOffset(MugenAnimation * tAnimation, MugenSpriteFile* tSprites);
int isMugenAnimationStepDurationInfinite(int tDuration);