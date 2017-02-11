#ifndef TARI_COLLISIONHANDLER_H
#define TARI_COLLISIONHANDLER_H

#include "collision.h"

typedef void (*CollisionCallback)(void* tCaller, void* tCollisionData);

void setupCollisionHandler();
void shutdownCollisionHandler();
void updateCollisionHandler();

int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData);
void addCollisionHandlerCheck(int tListID1, int tListID2);
int addCollisionListToHandler();

#endif
