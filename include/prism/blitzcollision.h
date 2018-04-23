#pragma once

#include <prism/actorhandler.h>
#include <prism/collision.h>

extern ActorBlueprint BlitzCollisionHandler;

void addBlitzCollisionComponent(int tEntityID);
int addBlitzCollisionPassiveMugen(int tEntityID, int tList);
int addBlitzCollisionAttackMugen(int tEntityID, int tList);
int addBlitzCollisionRect(int tEntityID, int tList, CollisionRect tRectangle);
int addBlitzCollisionCirc(int tEntityID, int tList, CollisionCirc tCircle);
void addBlitzCollisionCB(int tEntityID, int tCollisionID, void(*tCB)(void *, void*), void* tCaller);
void setBlitzCollisionCollisionData(int tEntityID, int tCollisionID, void* tCollisionData);
void setBlitzCollisionSolid(int tEntityID, int tCollisionID, int tIsMovable);

int hasBlitzCollidedTop(int tEntityID);
int hasBlitzCollidedBottom(int tEntityID);
int hasBlitzCollidedLeft(int tEntityID);
int hasBlitzCollidedRight(int tEntityID);

void removeAllBlitzCollisions(int tEntityID);