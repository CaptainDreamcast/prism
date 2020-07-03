#pragma once

#include <map>

#include "collision.h"

typedef void (*CollisionCallback)(void* tCaller, void* tCollisionData, int tOtherCollisionList);

struct CollisionListElement {
	int mID;
	int mListID;

	int mIsColliderOwned;
	Collider mCollider;

	CollisionCallback mCB;
	void* mCaller;

	void* mCollisionData;

	int mIsScheduledForDeletion;

};

struct CollisionListData {
	int mID;
	std::map<int, CollisionListElement> mCollisionElements;
};

void setupCollisionHandler();
void shutdownCollisionHandler();
void updateCollisionHandler();

CollisionListElement* addCollisionRectangleToCollisionHandler(CollisionListData* tList, Position* tBasePosition, const CollisionRect& tRect, CollisionCallback tCB, void* tCaller, void* tCollisionData);
void changeCollisionRectangleInCollisionHandler(CollisionListElement* tElement, const CollisionRect& tRect);
CollisionListElement* addCollisionCircleToCollisionHandler(CollisionListData* tList, Position* tBasePosition, const CollisionCirc& tCirc, CollisionCallback tCB, void* tCaller, void* tCollisionData);
CollisionListElement* addColliderToCollisionHandler(CollisionListData* tList, Position* tBasePosition, const Collider& tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData);
void addCollisionHandlerCheck(CollisionListData* tList1, CollisionListData* tList2);
CollisionListData* addCollisionListToHandler();
void removeFromCollisionHandler(CollisionListElement* tElement);
void updateColliderForCollisionHandler(CollisionListElement* tElement, const Collider& tCollider);
void setCollisionHandlerOwningColliders();
void resolveHandledCollisionMovableStatic(CollisionListElement* tElement1, CollisionListElement* tElement2, Position* tPos1, const Velocity& tVel1);

int isHandledCollisionAboveOtherCollision(CollisionListElement* tElement1, CollisionListElement* tElement2);
int isHandledCollisionBelowOtherCollision(CollisionListElement* tElement1, CollisionListElement* tElement2);
int isHandledCollisionLeftOfOtherCollision(CollisionListElement* tElement1, CollisionListElement* tElement2);
int isHandledCollisionRightOfOtherCollision(CollisionListElement* tElement1, CollisionListElement* tElement2);
int isHandledCollisionValid(CollisionListElement* tElement);
int isHandledCollisionScheduledForDeletion(CollisionListElement* tElement);

void drawColliderSolid(const Collider& tCollider, const Position& tOffset, const Position& tScreenPositionOffset, const Vector3D& tColor, double tAlpha);
void setCollisionHandlerDebuggingScreenPositionReference(Position* tPosition);
void drawHandledCollisions();
void activateCollisionHandlerDebugMode();

