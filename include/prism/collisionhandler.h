#pragma once

#include "collision.h"

typedef void (*CollisionCallback)(void* tCaller, void* tCollisionData);

void setupCollisionHandler();
void shutdownCollisionHandler();
void updateCollisionHandler();

int addCollisionRectangleToCollisionHandler(int tListID, Position* tBasePosition, CollisionRect tRect, CollisionCallback tCB, void* tCaller, void* tCollisionData);
int addCollisionCircleToCollisionHandler(int tListID, Position* tBasePosition, CollisionCirc tCirc, CollisionCallback tCB, void* tCaller, void* tCollisionData);
int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData);
void addCollisionHandlerCheck(int tListID1, int tListID2);
int addCollisionListToHandler();
void removeFromCollisionHandler(int tListID, int tElementID);
void updateColliderForCollisionHandler(int tListID, int tElementID, Collider tCollider);
void setCollisionHandlerOwningColliders();
void resolveHandledCollisionMovableStatic(int tListID1, int tElementID1, int tListID2, int tElementID2, Position* tPos1, Velocity tVel1);

int isHandledCollisionAboveOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2);
int isHandledCollisionBelowOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2);
int isHandledCollisionLeftOfOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2);
int isHandledCollisionRightOfOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2);

void setCollisionHandlerDebuggingScreenPositionReference(Position* tPosition);
void drawHandledCollisions();
void activateCollisionHandlerDebugMode();

