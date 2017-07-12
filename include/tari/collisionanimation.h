#ifndef TARI_COLLISION_ANIMATION_H
#define TARI_COLLISION_ANIMATION_H

#include "animation.h"
#include "collision.h"
#include "datastructures.h"
#include "collisionhandler.h"

typedef struct {
	Vector mFrames; // holds Colliders // TODO: rename

	Animation mAnimation;
} CollisionAnimation;

fup CollisionAnimation makeEmptyCollisionAnimation();
fup void destroyCollisionAnimation(CollisionAnimation* tAnimation);

fup AnimationResult updateCollisionAnimation(CollisionAnimation* tAnimation);
fup void resetCollisionAnimation(CollisionAnimation* tAnimation);
fup Collider getCollisionAnimationCollider(CollisionAnimation* tAnimation);


fup void setupCollisionAnimationHandler();
fup void shutdownCollisionAnimationHandler();

fup void updateCollisionAnimationHandler();
fup int addHandledCollisionAnimation(int tListID, Position* tBasePosition, CollisionAnimation tAnimation, CollisionCallback tCB, void* tCaller, void* tCollisionData);
fup void removeHandledCollisionAnimation(int tID);

fup void invertCollisionAnimationVertical(int tID);
fup void setCollisionAnimationCenter(int tID, Position tCenter);

#endif
