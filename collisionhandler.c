#include "tari/collisionhandler.h"

#include <stdlib.h>
#include <stdio.h>

#include "tari/datastructures.h"
#include "tari/memoryhandler.h"
#include "tari/drawing.h"
#include "tari/log.h"
#include "tari/system.h"

typedef struct {
	int mListID;
	Collider mCollider;

	CollisionCallback mCB;
	void* mCaller;

	void* mCollisionData;

	int mIsScheduledForDeletion;

} CollisionListElement;

typedef struct {
	IntMap mCollisionElements;
} CollisionListData;

typedef struct {

	int mID1;
	int mID2;

} CollisionListPair;

typedef struct {
	int mIsActive;
	
	Position* mScreenPositionReference;

	TextureData mCollisionRectTexture;
	TextureData mCollisionCircTexture;

} CollisionHandlerDebugData;

static struct {
	IntMap mCollisionLists;
	IntMap mCollisionListPairs;

	CollisionHandlerDebugData mDebug;

	int mIsOwningColliders;
} gData;

void setupCollisionHandler() {
	gData.mCollisionLists = new_int_map();
	gData.mCollisionListPairs = new_int_map();
	gData.mIsOwningColliders = 0;
}

static void destroyCollisionElement(CollisionListElement* e) {
	if (gData.mIsOwningColliders) {
		destroyCollider(&e->mCollider);
	}
}

static void cleanSingleCollisionListElement(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListElement* data = tData;
	destroyCollisionElement(data);
}

static void cleanSingleCollisionList(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListData* data = tData;
	int_map_map(&data->mCollisionElements, cleanSingleCollisionListElement, NULL);
	int_map_empty(&data->mCollisionElements);
}

void shutdownCollisionHandler() {
	int_map_map(&gData.mCollisionLists, cleanSingleCollisionList, NULL);
	int_map_empty(&gData.mCollisionLists);
	int_map_empty(&gData.mCollisionListPairs);
}

static void checkCollisionElements(void* tCaller, void* tData) {
	CollisionListElement* e1 = tCaller;
	CollisionListElement* e2 = tData;

	if (e1->mIsScheduledForDeletion || e2->mIsScheduledForDeletion) return;

	if(checkCollisionCollider(e1->mCollider, e2->mCollider)) {
		if(e1->mCB) e1->mCB(e1->mCaller, e2->mCollisionData);
		if(e2->mCB) e2->mCB(e2->mCaller, e1->mCollisionData);
	}
}

static void checkAgainstOtherList(void* tCaller, void *tData) {
	IntMap* list = tCaller;
	CollisionListElement* e = tData;

	if (e->mIsScheduledForDeletion) return;

	int_map_map(list, checkCollisionElements, e);
}

static int checkElementRemoval(void* tCaller, void * tData) {
	(void) tCaller;
	CollisionListElement* e = tData;
	
	if (e->mIsScheduledForDeletion) {
		destroyCollisionElement(e);
	}

	return e->mIsScheduledForDeletion;
}

static void updateSingleCollisionPair(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListPair* data = tData;

	CollisionListData* list1 = int_map_get(&gData.mCollisionLists, data->mID1);
	CollisionListData* list2 = int_map_get(&gData.mCollisionLists, data->mID2);

	int_map_remove_predicate(&list1->mCollisionElements, checkElementRemoval, NULL);
	int_map_remove_predicate(&list2->mCollisionElements, checkElementRemoval, NULL);

	int_map_map(&list1->mCollisionElements, checkAgainstOtherList, list2);
	
	int_map_remove_predicate(&list1->mCollisionElements, checkElementRemoval, NULL);	
	int_map_remove_predicate(&list2->mCollisionElements, checkElementRemoval, NULL);

}

void updateCollisionHandler() {
	int_map_map(&gData.mCollisionListPairs, updateSingleCollisionPair, NULL);
}


int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	CollisionListData* list = int_map_get(&gData.mCollisionLists, tListID);
	
	CollisionListElement* e = allocMemory(sizeof(CollisionListElement));
	e->mListID = tListID;

	e->mCollider = tCollider;
	setColliderBasePosition(&e->mCollider, tBasePosition);

	e->mCB = tCB;
	e->mCaller = tCaller;

	e->mCollisionData = tCollisionData;

	e->mIsScheduledForDeletion = 0;

	return int_map_push_back_owned(&list->mCollisionElements, e);
}


void addCollisionHandlerCheck(int tListID1, int tListID2) {
	CollisionListPair* e = allocMemory(sizeof(CollisionListPair));
	e->mID1 = tListID1;
	e->mID2 = tListID2;

	int_map_push_back_owned(&gData.mCollisionListPairs, e);
}

int addCollisionListToHandler() {
	CollisionListData* e = allocMemory(sizeof(CollisionListData));
	e->mCollisionElements = new_int_map();

	return int_map_push_back_owned(&gData.mCollisionLists, e);
}

void removeFromCollisionHandler(int tListID, int tElementID) {
	CollisionListData* list = int_map_get(&gData.mCollisionLists, tListID);
	CollisionListElement* e = int_map_get(&list->mCollisionElements, tElementID);
	e->mIsScheduledForDeletion = 1;
}

void updateColliderForCollisionHandler(int tListID, int tElementID, Collider tCollider) {
	CollisionListData* list = int_map_get(&gData.mCollisionLists, tListID);
	CollisionListElement* e = int_map_get(&list->mCollisionElements, tElementID);

	e->mCollider = tCollider;
}

void setCollisionHandlerOwningColliders() {
	gData.mIsOwningColliders = 1;
}

#define DEBUG_Z 12

static void drawCollisionRect(CollisionRect tRect, Position* tBasePosition){
	double dx = tRect.mBottomRight.x -  tRect.mTopLeft.x;
	double dy = tRect.mBottomRight.y -  tRect.mTopLeft.y;
	
	if(dx < 0 || dy < 0) return;

	Position position = vecAdd(tRect.mTopLeft, *tBasePosition);
	
	if(gData.mDebug.mScreenPositionReference != NULL) {	
		position = vecAdd(position, vecScale(*gData.mDebug.mScreenPositionReference, -1));
	}

	position.z = DEBUG_Z;
	
	Vector3D scale = makePosition(dx / 16.0, dy / 16.0, 1);
	scaleDrawing3D(scale, position);

	drawSprite(gData.mDebug.mCollisionRectTexture, position, makeRectangleFromTexture(gData.mDebug.mCollisionRectTexture));
	setDrawingParametersToIdentity();

	
}

static void drawCollisionCirc(CollisionCirc tCirc, Position* tBasePosition) {
	double r = tCirc.mRadius;
	double d = r * 2;

	if (r < 0) return;

	Position position = vecAdd(tCirc.mCenter, *tBasePosition);
	position = vecAdd(position, vecScale(makePosition(r, r, 0), -1));

	if (gData.mDebug.mScreenPositionReference != NULL) {
		position = vecAdd(position, vecScale(*gData.mDebug.mScreenPositionReference, -1));
	}

	position.z = DEBUG_Z;

	Vector3D scale = makePosition(d / 16.0, d / 16.0, 1);
	scaleDrawing3D(scale, position);

	drawSprite(gData.mDebug.mCollisionCircTexture, position, makeRectangleFromTexture(gData.mDebug.mCollisionCircTexture));
	setDrawingParametersToIdentity();
}

static void drawCollisionElement(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListElement* e = tData;
	
	Collider col = e->mCollider;

	if(col.mType == COLLISION_RECT) {
		CollisionRect* rect = col.mData;
		drawCollisionRect(*rect, col.mBasePosition);
	} else if (col.mType == COLLISION_CIRC) {
		CollisionCirc* circ = col.mData;
		drawCollisionCirc(*circ, col.mBasePosition);
	}
	else {
		logError("Unable to draw collision type");
		logErrorInteger(col.mType);
		abortSystem();
	}
}

static void drawCollisionList(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListData* list = tData;
	
	int_map_map(&list->mCollisionElements, drawCollisionElement, NULL);
}

void setCollisionHandlerDebuggingScreenPositionReference(Position* tPosition) {
	gData.mDebug.mScreenPositionReference = tPosition;
}

void drawHandledCollisions() {
	if(!gData.mDebug.mIsActive) return;

	int_map_map(&gData.mCollisionLists, drawCollisionList, NULL);
}

void activateCollisionHandlerDebugMode() {
	gData.mDebug.mIsActive = 1;
	gData.mDebug.mScreenPositionReference = NULL;	

	gData.mDebug.mCollisionRectTexture = loadTexturePKG("$/rd/debug/collision_rect.pkg");
	gData.mDebug.mCollisionCircTexture = loadTexturePKG("$/rd/debug/collision_circ.pkg");

}
