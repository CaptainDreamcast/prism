#pragma once

#include <prism/actorhandler.h>
#include <prism/collision.h>

struct CollisionListData;
struct CollisionListElement;

ActorBlueprint getBlitzCollisionHandler();

void addBlitzCollisionComponent(int tEntityID);
int addBlitzCollisionPassiveMugen(int tEntityID, CollisionListData* tList);
int addBlitzCollisionAttackMugen(int tEntityID, CollisionListData* tList);
int addBlitzCollisionRect(int tEntityID, CollisionListData* tList, const CollisionRect& tRectangle);
void changeBlitzCollisionRect(int tEntityID, int tCollisionID, const CollisionRect& tRectangle);
int addBlitzCollisionCirc(int tEntityID, CollisionListData* tList, const CollisionCirc& tCircle);
void addBlitzCollisionCB(int tEntityID, int tCollisionID, void(*tCB)(void *, void*), void* tCaller);
void setBlitzCollisionCollisionData(int tEntityID, int tCollisionID, void* tCollisionData);
void setBlitzCollisionSolid(int tEntityID, int tCollisionID, int tIsMovable);
void setBlitzCollisionUnsolid(int tEntityID, int tCollisionID);

int hasBlitzCollidedTop(int tEntityID);
int hasBlitzCollidedBottom(int tEntityID);
int hasBlitzCollidedLeft(int tEntityID);
int hasBlitzCollidedRight(int tEntityID);

void removeAllBlitzCollisions(int tEntityID);