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

fup void setupAnimationTreeHandling();
fup void shutdownAnimationTreeHandling();
fup void updateAnimationTreeHandling();
fup void drawAnimationTreeHandling();

fup AnimationTree loadAnimationTree(char* tPath);

fup int playAnimationTreeLoop(Position tPosition, AnimationTree tTree, char* tAnimation);
fup void setAnimationTreeAnimation(AnimationTree* tTree, char* tAnimation);
fup void setHandledAnimationTreeAnimation(int tID, char* tAnimation);