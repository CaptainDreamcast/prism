#include "prism/collisionhandler.h"

#include <stdlib.h>
#include <stdio.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/drawing.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/screeneffect.h"
#include "prism/geometry.h"

typedef struct {
	int mListID;
	
	int mIsColliderOwned;
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
	if (gData.mIsOwningColliders || e->mIsColliderOwned) {
		destroyCollider(&e->mCollider);
	}
}

static void cleanSingleCollisionListElement(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListElement* data = (CollisionListElement*)tData;
	destroyCollisionElement(data);
}

static void cleanSingleCollisionList(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListData* data = (CollisionListData*)tData;
	int_map_map(&data->mCollisionElements, cleanSingleCollisionListElement, NULL);
	delete_int_map(&data->mCollisionElements);
}

void shutdownCollisionHandler() {
	int_map_map(&gData.mCollisionLists, cleanSingleCollisionList, NULL);
	delete_int_map(&gData.mCollisionLists);
	delete_int_map(&gData.mCollisionListPairs);
}

static void checkCollisionElements(void* tCaller, void* tData) {
	CollisionListElement* e1 = (CollisionListElement*)tCaller;
	CollisionListElement* e2 = (CollisionListElement*)tData;

	if (e1->mIsScheduledForDeletion || e2->mIsScheduledForDeletion) return;

	if(checkCollisionCollider(e1->mCollider, e2->mCollider)) {
		if(e1->mCB) e1->mCB(e1->mCaller, e2->mCollisionData);
		if(e2->mCB) e2->mCB(e2->mCaller, e1->mCollisionData);
	}
}

static void checkAgainstOtherList(void* tCaller, void *tData) {
	IntMap* list = (IntMap*)tCaller;
	CollisionListElement* e = (CollisionListElement*)tData;

	if (e->mIsScheduledForDeletion) return;

	int_map_map(list, checkCollisionElements, e);
}

static int checkElementRemoval(void* tCaller, void * tData) {
	(void) tCaller;
	CollisionListElement* e = (CollisionListElement*)tData;
	
	if (e->mIsScheduledForDeletion) {
		destroyCollisionElement(e);
	}

	return e->mIsScheduledForDeletion;
}

static void updateSingleCollisionPair(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListPair* data = (CollisionListPair*)tData;

	CollisionListData* list1 = (CollisionListData*)int_map_get(&gData.mCollisionLists, data->mID1);
	CollisionListData* list2 = (CollisionListData*)int_map_get(&gData.mCollisionLists, data->mID2);

	int_map_remove_predicate(&list1->mCollisionElements, checkElementRemoval, NULL);
	int_map_remove_predicate(&list2->mCollisionElements, checkElementRemoval, NULL);

	int_map_map(&list1->mCollisionElements, checkAgainstOtherList, list2);
	
	int_map_remove_predicate(&list1->mCollisionElements, checkElementRemoval, NULL);	
	int_map_remove_predicate(&list2->mCollisionElements, checkElementRemoval, NULL);

}

void updateCollisionHandler() {
	int_map_map(&gData.mCollisionListPairs, updateSingleCollisionPair, NULL);
}

static void setColliderOwned(int tListID, int tID) {
	CollisionListData* list = (CollisionListData*)int_map_get(&gData.mCollisionLists, tListID);
	CollisionListElement* e = (CollisionListElement*)int_map_get(&list->mCollisionElements, tID);

	e->mIsColliderOwned = 1;
}

int addCollisionRectangleToCollisionHandler(int tListID, Position* tBasePosition, CollisionRect tRect, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	Collider collider = makeColliderFromRect(tRect);
	int id = addColliderToCollisionHandler(tListID, tBasePosition, collider, tCB, tCaller, tCollisionData);
	setColliderOwned(tListID, id);
	return id;
}

int addCollisionCircleToCollisionHandler(int tListID, Position* tBasePosition, CollisionCirc tCirc, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	Collider collider = makeColliderFromCirc(tCirc);
	int id = addColliderToCollisionHandler(tListID, tBasePosition, collider, tCB, tCaller, tCollisionData);
	setColliderOwned(tListID, id);
	return id;
}

int addColliderToCollisionHandler(int tListID, Position* tBasePosition, Collider tCollider, CollisionCallback tCB, void* tCaller, void* tCollisionData) {
	CollisionListData* list = (CollisionListData*)int_map_get(&gData.mCollisionLists, tListID);
	
	CollisionListElement* e = (CollisionListElement*)allocMemory(sizeof(CollisionListElement));
	e->mListID = tListID;

	e->mCollider = tCollider;
	setColliderBasePosition(&e->mCollider, tBasePosition);
	e->mIsColliderOwned = 0;

	e->mCB = tCB;
	e->mCaller = tCaller;

	e->mCollisionData = tCollisionData;

	e->mIsScheduledForDeletion = 0;

	return int_map_push_back_owned(&list->mCollisionElements, e);
}


void addCollisionHandlerCheck(int tListID1, int tListID2) {
	CollisionListPair* e = (CollisionListPair*)allocMemory(sizeof(CollisionListPair));
	e->mID1 = tListID1;
	e->mID2 = tListID2;

	int_map_push_back_owned(&gData.mCollisionListPairs, e);
}

int addCollisionListToHandler() {
	CollisionListData* e = (CollisionListData*)allocMemory(sizeof(CollisionListData));
	e->mCollisionElements = new_int_map();

	return int_map_push_back_owned(&gData.mCollisionLists, e);
}

void removeFromCollisionHandler(int tListID, int tElementID) {
	CollisionListData* list = (CollisionListData*)int_map_get(&gData.mCollisionLists, tListID);
	CollisionListElement* e = (CollisionListElement*)int_map_get(&list->mCollisionElements, tElementID);
	e->mIsScheduledForDeletion = 1;
}

void updateColliderForCollisionHandler(int tListID, int tElementID, Collider tCollider) {
	CollisionListData* list = (CollisionListData*)int_map_get(&gData.mCollisionLists, tListID);
	CollisionListElement* e = (CollisionListElement*)int_map_get(&list->mCollisionElements, tElementID);

	if (e->mIsColliderOwned) {
		destroyCollider(&e->mCollider);
	}

	e->mCollider = tCollider;
}

void setCollisionHandlerOwningColliders() {
	gData.mIsOwningColliders = 1;
}

static CollisionListElement* getCollisionListElement(int tListID, int tElementID) {
	if (!int_map_contains(&gData.mCollisionLists, tListID)) {
		logErrorFormat("Collision handler does not contain list with id %d.", tListID);
		recoverFromError();
	}
	CollisionListData* list = (CollisionListData*)int_map_get(&gData.mCollisionLists, tListID);

	if (!int_map_contains(&list->mCollisionElements, tElementID)) {
		logErrorFormat("Collision handler list %d does not contain element with id %d.", tListID, tElementID);
		recoverFromError();
	}
	return (CollisionListElement*)int_map_get(&list->mCollisionElements, tElementID);
}

void resolveHandledCollisionMovableStatic(int tListID1, int tElementID1, int tListID2, int tElementID2, Position* tPos1, Velocity tVel1)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);	
	resolveCollisionColliderColliderMovableStatic(tPos1, tVel1, e1->mCollider, e2->mCollider);
}

int isHandledCollisionAboveOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderDown(e1->mCollider) <= getColliderUp(e2->mCollider);
}

int isHandledCollisionBelowOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderUp(e1->mCollider) >= getColliderDown(e2->mCollider);
}

int isHandledCollisionLeftOfOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderRight(e1->mCollider) <= getColliderLeft(e2->mCollider);
}

int isHandledCollisionRightOfOtherCollision(int tListID1, int tElementID1, int tListID2, int tElementID2)
{
	CollisionListElement* e1 = getCollisionListElement(tListID1, tElementID1);
	CollisionListElement* e2 = getCollisionListElement(tListID2, tElementID2);
	return getColliderLeft(e1->mCollider) >= getColliderRight(e2->mCollider);
}


#define DEBUG_Z 99

static void drawCollisionRect(CollisionRect tRect, Position tBasePosition, Position tScreenPositionOffset, Vector3D tColor, double tAlpha){
	double dx = tRect.mBottomRight.x -  tRect.mTopLeft.x;
	double dy = tRect.mBottomRight.y -  tRect.mTopLeft.y;
	
	if(dx < 0 || dy < 0) return;

	Position position = vecAdd(tRect.mTopLeft, tBasePosition);
	position = vecSub(position, tScreenPositionOffset);
	position.z = DEBUG_Z;
	
	Vector3D scale = makePosition(dx / 16.0, dy / 16.0, 1);
	scaleDrawing3D(scale, position);
	
	setDrawingBaseColorAdvanced(tColor.x, tColor.y, tColor.z);
	setDrawingTransparency(tAlpha);

	TextureData whiteTexture = getEmptyWhiteTexture();
	drawSprite(whiteTexture, position, makeRectangleFromTexture(whiteTexture));
	setDrawingParametersToIdentity();

	
}

static void drawCollisionCirc(CollisionCirc tCirc, Position tBasePosition, Position tScreenPositionOffset, Vector3D tColor, double tAlpha) {
	double r = tCirc.mRadius;
	double d = r * 2;

	if (r < 0) return;

	Position position = vecAdd(tCirc.mCenter, tBasePosition);
	position = vecAdd(position, vecScale(makePosition(r, r, 0), -1));
	position = vecSub(position, tScreenPositionOffset);
	position.z = DEBUG_Z;

	Vector3D scale = makePosition(d / 16.0, d / 16.0, 1);
	scaleDrawing3D(scale, position);
	
	setDrawingBaseColorAdvanced(tColor.x, tColor.y, tColor.z);
	setDrawingTransparency(tAlpha);

	TextureData whiteTexture = getEmptyWhiteTexture();
	drawSprite(whiteTexture, position, makeRectangleFromTexture(whiteTexture)); // TODO: circle texture
	setDrawingParametersToIdentity();
}

void drawColliderSolid(Collider tCollider, Position tOffset, Position tScreenPositionOffset, Vector3D tColor, double tAlpha) {
	Position basePosition;
	if (tCollider.mBasePosition) basePosition = *tCollider.mBasePosition;
	else basePosition = makePosition(0, 0, 0);

	basePosition = vecAdd(basePosition, tOffset);

	if (tCollider.mType == COLLISION_RECT) {
		CollisionRect* rect = (CollisionRect*)tCollider.mData;
		drawCollisionRect(*rect, basePosition, tScreenPositionOffset, tColor, tAlpha);
	}
	else if (tCollider.mType == COLLISION_CIRC) {
		CollisionCirc* circ = (CollisionCirc*)tCollider.mData;
		drawCollisionCirc(*circ, basePosition, tScreenPositionOffset, tColor, tAlpha);
	}
	else {
		logError("Unable to draw collision type");
		logErrorInteger(tCollider.mType);
		recoverFromError();
	}
}

static void drawCollisionElement(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListElement* e = (CollisionListElement*)tData;
	
	Collider col = e->mCollider;

	Position screenOffset;
	if (gData.mDebug.mScreenPositionReference) screenOffset = *gData.mDebug.mScreenPositionReference;
	else screenOffset = makePosition(0, 0, 0);

	drawColliderSolid(col, makePosition(0, 0, 0), screenOffset, makePosition(1, 1, 1), 1);
}

static void drawCollisionList(void* tCaller, void* tData) {
	(void) tCaller;
	CollisionListData* list = (CollisionListData*)tData;
	
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
}
