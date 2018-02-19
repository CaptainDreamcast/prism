#pragma once

#include "collision.h"

typedef void (*CollisionCallback)(void* tCaller, void* tCollisionData);

void setupCollisionHandler();
void shutdownCollisionHandler();
void updateCollisionHandler();

int addCollisionRectangleToCollisionHandler(int tListID, Position* tBasePosition, CollisionRect tRect, CollisionCallback tCB, void* tCaller, void* tCollisionData);
int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData);
void addCollisionHandlerCheck(int tListID1, int tListID2);
int addCollisionListToHandler();
void removeFromCollisionHandler(int tListID, int tElementID);
void updateColliderForCollisionHandler(int tListID, int tElementID, Collider tCollider);
void setCollisionHandlerOwningColliders();

void setCollisionHandlerDebuggingScreenPositionReference(Position* tPosition);
void drawHandledCollisions();
void activateCollisionHandlerDebugMode();

