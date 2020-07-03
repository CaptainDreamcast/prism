#pragma once

#include "geometry.h"
#include "datastructures.h"
#include "texture.h"
#include "animation.h"

typedef struct {
	Position mAnchor;
	Position mTranslation;
	double mRotationZ;

	TextureData* mTextures;
	Animation mAnimation;

	StringMap mSubElements;
} TreeNode;

typedef struct{
	TreeNode mTree;
	StringMap mAnimations;

	char mCurrentAnimation[100];
} AnimationTree;

void setupAnimationTreeHandling();
void shutdownAnimationTreeHandling();
void updateAnimationTreeHandling();
void drawAnimationTreeHandling();

AnimationTree loadAnimationTree(char* tPath);

int playAnimationTreeLoop(const Position& tPosition, const AnimationTree& tTree, char* tAnimation);
void setAnimationTreeAnimation(AnimationTree* tTree, char* tAnimation);
void setHandledAnimationTreeAnimation(int tID, char* tAnimation);