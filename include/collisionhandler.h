#ifndef TARI_COLLISIONHANDLER_H
#define TARI_COLLISIONHANDLER_H

#include "collision.h"

typedef void (*CollisionCallback)(void* tCaller, void* tCollisionData);

fup void setupCollisionHandler();
fup void shutdownCollisionHandler();
fup void updateCollisionHandler();

fup int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData);
fup void addCollisionHandlerCheck(int tListID1, int tListID2);
fup int addCollisionListToHandler();
fup void removeFromCollisionHandler(int tListID, int tElementID);
fup void updateColliderForCollisionHandler(int tListID, int tElementID, Collider tCollider);

fup void setCollisionHandlerDebuggingScreenPositionReference(Position* tPosition);
fup void drawHandledCollisions();
fup void activateCollisionHandlerDebugMode();

#endif
