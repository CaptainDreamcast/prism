#ifndef TARI_COLLISION_ANIMATION_H
#define TARI_COLLISION_ANIMATION_H

#include "animation.h"
#include "collision.h"
#include "datastructures.h"
#include "collisionhandler.h"

typedef struct {
	Vector mFrames;

	Animation mAnimation;
} CollisionAnimation;

CollisionAnimation makeEmptyCollisionAnimation();
void destroyCollisionAnimation(CollisionAnimation* tAnimation);

AnimationResult updateCollisionAnimation(CollisionAnimation* tAnimation);
void resetCollisionAnimation(CollisionAnimation* tAnimation);
Collider getCollisionAnimationCollider(CollisionAnimation* tAnimation);


void setupCollisionAnimationHandler();
void shutdownCollisionAnimationHandler();

void updateCollisionAnimationHandler();
int addHandledCollisionAnimation(int tListID, Position* tBasePosition, CollisionAnimation tAnimation, CollisionCallback tCB, void* tCaller, void* tCollisionData);
void removeHandledCollisionAnimation(int tID);

void invertCollisionAnimationVertical(int tID);
void setCollisionAnimationCenter(int tID, Position tCenter);

#endif
