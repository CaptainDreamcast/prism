#pragma once

#include "animation.h"
#include "collision.h"
#include "datastructures.h"
#include "collisionhandler.h"
#include "actorhandler.h"

typedef struct {
	Vector mFrames; // holds Colliders

	Animation mAnimation;
} CollisionAnimation;

CollisionAnimation makeEmptyCollisionAnimation();
void destroyCollisionAnimation(CollisionAnimation* tAnimation);

AnimationResult updateCollisionAnimation(CollisionAnimation* tAnimation);
void resetCollisionAnimation(CollisionAnimation* tAnimation);
Collider getCollisionAnimationCollider(CollisionAnimation* tAnimation);

ActorBlueprint getCollisionAnimationHandler();

int addHandledCollisionAnimation(CollisionListData* tListID, Position* tBasePosition, const CollisionAnimation& tAnimation, CollisionCallback tCB, void* tCaller, void* tCollisionData);
void removeHandledCollisionAnimation(int tID);

void invertCollisionAnimationVertical(int tID);
void setCollisionAnimationCenter(int tID, const Position& tCenter);
